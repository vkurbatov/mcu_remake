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

#define RFB_PIXEL_FORMAT 8, 3, 4

namespace vnc
{

std::uint32_t owner_tag = 0xADAF;

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
    }

    bool init(const vnc_server_config_t& config)
    {
        rfb_client = rfbGetClient(RFB_PIXEL_FORMAT);

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


    std::size_t fetch_frame(frame_t& frame
                            , std::uint32_t timeout = 0)
    {
        if (is_init)
        {
            auto result = WaitForMessage(rfb_client, timeout * 1000);

            if(result >= 0
                    && HandleRFBServerMessage(rfb_client))
            {
                std::lock_guard<std::mutex> lg(mutex);
                frame = this->frame;

                return frame_counter;
            }
        }

        return 0;
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

    vnc_device_context_t(frame_handler_t frame_handler)
        : m_frame_handler(frame_handler)
        , m_running(false)
        , m_established(false)
    {

    }

    bool open(const vnc_server_config_t &server_config)
    {
        close();


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

    void stream_proc()
    {

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

bool vnc_device::open(const vnc_server_config_t &server_config)
{
    return m_vnc_device_context->open(server_config);
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


}
