#include "vnc_device.h"

extern "C"
{
#include <rfb/rfbclient.h>
#include <rfb/rfbproto.h>
};

#include <sys/ioctl.h>

#include <cstring>
#include <mutex>
#include <thread>
#include <atomic>
#include <iostream>
#include <cstdarg>

#define RFB_PIXEL_FORMAT_DEFAULT 8, 3, 4

#define RFB_PIXEL_FORMAT_32 8, 3, 4
#define RFB_PIXEL_FORMAT_24 8, 3, 3
#define RFB_PIXEL_FORMAT_16 6, 16, 16
#define RFB_PIXEL_FORMAT_15 4, 15, 16
#define RFB_PIXEL_FORMAT_8 3, 8, 8

namespace vnc
{

std::uint32_t owner_tag = 0xADAF;
const std::size_t max_queue_size = 100;


void rfb_logging(const char* fmt, ...)
{
    std::array<char, 1024> buffer = {0};

    va_list args;
    va_start(args, fmt);
    int written = vsnprintf(&buffer[0], 1024, fmt, args);
    va_end(args);

    std::string out_str(&buffer[0]);
    size_t nl_pos = out_str.find('\n');

    if (nl_pos != std::string::npos)
    {
        out_str[nl_pos] = ' ';
    }

    std::cout << "vnc_client: \t" << out_str << std::endl;
}

static bool connect(rfbClient& rfb_client)
{
    if (!rfb_client.listenSpecified)
    {
        if (!rfb_client.serverHost)
        {
            return false;
        }

        if (rfb_client.destHost != nullptr)
        {
            if (!ConnectToRFBRepeater(&rfb_client
                                      , rfb_client.serverHost
                                      , rfb_client.serverPort
                                      , rfb_client.destHost
                                      , rfb_client.destPort))
            {
                return false;
            }
        }
        else
        {
            if (!ConnectToRFBServer(&rfb_client
                                    , rfb_client.serverHost
                                    , rfb_client.serverPort))
            {
                return false;
            }
        }
    }

    if (InitialiseRFBConnection(&rfb_client))
    {
        rfb_client.width = rfb_client.si.framebufferWidth;
        rfb_client.height = rfb_client.si.framebufferHeight;
        rfb_client.MallocFrameBuffer(&rfb_client);

        return true;
    }

    return false;
}

static bool set_scale_setting(rfbClient& rfb_client)
{
    if (rfb_client.updateRect.x < 0)
    {
        rfb_client.updateRect.x = 0;
        rfb_client.updateRect.y = 0;
        rfb_client.updateRect.w = rfb_client.width;
        rfb_client.updateRect.h = rfb_client.height;
    }

    if (rfb_client.appData.scaleSetting > 1)
    {
        if (!SendScaleSetting(&rfb_client
                              , rfb_client.appData.scaleSetting))
        {
            return false;
        }

        if (!SendFramebufferUpdateRequest(&rfb_client,
                                          rfb_client.updateRect.x / rfb_client.appData.scaleSetting,
                                          rfb_client.updateRect.y / rfb_client.appData.scaleSetting,
                                          rfb_client.updateRect.w / rfb_client.appData.scaleSetting,
                                          rfb_client.updateRect.h / rfb_client.appData.scaleSetting,
                                          false))
        {
            return false;
        }
    }
    else
    {
        if (!SendFramebufferUpdateRequest(&rfb_client,
                                          rfb_client.updateRect.x, rfb_client.updateRect.y,
                                          rfb_client.updateRect.w, rfb_client.updateRect.h,
                                          false))
        {
            return false;
        }
    }

    return true;
}

struct vnc_client_t
{
    rfbClient               *rfb_client;
    vnc_server_config_t     server_config;
    std::size_t             frame_counter;
    std::mutex              mutex;

    bool                    is_init;

    frame_t                 frame;

    vnc_client_t(const vnc_server_config_t& server_config)
        : rfb_client(nullptr)
        , server_config(server_config)
        , frame_counter(0)
        , is_init(false)
    {
        //rfbClientLog = rfbClientErr = rfb_logging;
        is_init = init(this->server_config);
    }

    ~vnc_client_t()
    {
        if (rfb_client != nullptr)
        {
            if (rfb_client->sock >= 0)
            {
                ::close(rfb_client->sock);
            }

            rfb_client->frameBuffer = nullptr;
            rfb_client->serverHost = nullptr;
        }

        rfbClientCleanup(rfb_client);
        rfb_client = nullptr;
    }

    bool init(const vnc_server_config_t& config)
    {
        rfb_client = rfbGetClient(RFB_PIXEL_FORMAT_32);

        if (rfb_client != nullptr)
        {
            rfb_client->MallocFrameBuffer = [](rfbClient* client) -> rfbBool
            {
                auto vnc_client = static_cast<vnc_client_t*>(rfbClientGetClientData(client
                                                                                    , &owner_tag));
                if (vnc_client != nullptr)
                {
                    std::lock_guard<std::mutex> lg(vnc_client->mutex);

                    vnc_client->frame.frame_size.width = client->width;
                    vnc_client->frame.frame_size.height = client->height;
                    vnc_client->frame.bpp = client->format.bitsPerPixel;
                    vnc_client->frame.realloc();
                    client->frameBuffer = vnc_client->frame.frame_data.data();

                    client->updateRect.x = 0;
                    client->updateRect.y = 0;
                    client->updateRect.w = client->width;
                    client->updateRect.h = client->height;

                    return SetFormatAndEncodings(client);
                }

                return false;
            };

            rfb_client->GotFrameBufferUpdate = [](rfbClient* client
                                                  , int x
                                                  , int y
                                                  , int width
                                                  , int height)
            {
                auto vnc_client = static_cast<vnc_client_t*>(rfbClientGetClientData(client
                                                                                    , &owner_tag));

                if (vnc_client != nullptr)
                {
                    vnc_client->frame_counter++;
                }
            };

            rfb_client->GetPassword = [](rfbClient* client) -> char*
            {
                auto vnc_client = static_cast<vnc_client_t*>(rfbClientGetClientData(client
                                                                                    , &owner_tag));

                if (vnc_client != nullptr)
                {
                    return strdup(vnc_client->server_config.password.c_str());
                }

                return nullptr;
            };

            rfb_client->canHandleNewFBSize = true;
            rfb_client->frameBuffer = nullptr;
            rfb_client->programName = nullptr;
            rfb_client->serverHost = const_cast<char*>(config.host.c_str());
            rfb_client->serverPort = config.port;           

            rfbClientSetClientData(rfb_client
                                   , &owner_tag
                                   , this);

            return connect(*rfb_client)
                    && set_scale_setting(*rfb_client);

        }

        return false;
    }


    io_result_t fetch_frame(frame_t& frame
                            , std::uint32_t timeout = 0)
    {
        if (is_init)
        {
            auto result = WaitForMessage(rfb_client, timeout * 1000);

            if(result > 0
                    && HandleRFBServerMessage(rfb_client))
            {
                frame = this->frame;

                return io_result_t::complete;
            }

            return result == 0
                    ? io_result_t::timeout
                    : io_result_t::error;
        }
        return io_result_t::not_ready;
    }
};

struct vnc_device_context_t
{
    frame_handler_t                 m_frame_handler;

    std::thread                     m_stream_thread;
    mutable std::mutex              m_mutex;

    std::atomic_bool                m_running;
    std::atomic_bool                m_established;

    std::unique_ptr<vnc_client_t>   m_client;
    frame_queue_t                   m_frame_queue;

    vnc_device_context_t(frame_handler_t frame_handler)
        : m_frame_handler(frame_handler)
        , m_running(false)
        , m_established(false)
    {

    }

    bool open(const vnc_server_config_t &server_config
              , std::uint32_t fps)
    {
        close();
        m_running = true;
        m_stream_thread = std::thread(&vnc_device_context_t::stream_proc
                                      , this
                                      , server_config
                                      , fps);
    }

    bool close()
    {
        if (m_running == true)
        {
            m_running = false;
            if (m_stream_thread.joinable())
            {
                m_stream_thread.join();
                return true;
            }
        }

        return false;
    }

    void process_frame(frame_t&& frame)
    {
        if (m_frame_handler == nullptr
                || m_frame_handler(std::move(frame)) == false)
        {
            std::lock_guard<std::mutex> lg(m_mutex);
            m_frame_queue.emplace(std::move(frame));

            while (m_frame_queue.size() > max_queue_size)
            {
                m_frame_queue.pop();
            }
        }
    }

    frame_queue_t fetch_frame_queue()
    {
        std::lock_guard<std::mutex> lg(m_mutex);
        return std::move(m_frame_queue);
    }

    void stream_proc(const vnc_server_config_t &server_config
                     , std::uint32_t fps)
    {
        while (m_running)
        {
            {
                std::unique_ptr<vnc_client_t> vnc_client(new vnc_client_t(server_config));

                if (vnc_client->is_init)
                {
                    bool is_complete = false;

                    const auto frame_time = 1000 / fps;

                    while (m_running
                           && is_complete == false)
                    {
                        frame_t frame;
                        auto io_result = vnc_client->fetch_frame(frame
                                                                 , frame_time * 2);

                        switch (io_result)
                        {
                            case io_result_t::complete:
                            {
                                process_frame(std::move(frame));
                            }
                            break;
                            case io_result_t::timeout:
                                // nothing
                            break;
                            default:
                            {
                                is_complete = true;
                            }
                        }

                        if (is_complete == false)
                        {
                            std::this_thread::sleep_for(std::chrono::milliseconds(frame_time));
                        }
                    }
                }
            }

            if (m_running)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        }
    }
};

void vnc_device_context_deleter_t::operator()(vnc_device_context_t *vnc_device_context_ptr)
{
    delete vnc_device_context_ptr;
}

vnc_device::vnc_device(frame_handler_t frame_handler)
    : m_vnc_device_context(new vnc_device_context_t(frame_handler))
{

}

bool vnc_device::open(const vnc_server_config_t &server_config
                      , std::uint32_t fps)
{
    return m_vnc_device_context->open(server_config
                                      , fps);
}

bool vnc_device::open(const std::string &uri
                      , std::uint32_t fps)
{
    return m_vnc_device_context->open(vnc_server_config_t::from_uri(uri)
                                      , fps);
}

bool vnc_device::close()
{
    return m_vnc_device_context->close();
}

bool vnc_device::is_opened() const
{
    return m_vnc_device_context->m_running;
}

bool vnc_device::is_established() const
{
    return m_vnc_device_context->m_established;
}

frame_queue_t vnc_device::fetch_frame_queue()
{
    return std::move(m_vnc_device_context->fetch_frame_queue());
}


}
