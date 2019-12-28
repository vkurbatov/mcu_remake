#include "v4l2_device.h"
#include "v4l2_api.h"

#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <condition_variable>

namespace v4l2
{

struct v4l2_object_t
{
    std::int32_t handle;
    frame_info_t frame_info;
    mapped_buffer_t mapped_buffer;

    v4l2_object_t(const std::string& uri
                  , const frame_info_t& frame_info = frame_info_t()
                  , std::size_t buffer_count = 1)

        : handle(v4l2::open_device(uri))
        , frame_info(frame_info)
    {
        if (handle >= 0)
        {
            if (!frame_info.is_null())
            {
                set_frame_info(frame_info);
            }

            fetch_frame_info(this->frame_info);

            mapped_buffer = std::move(std::move(v4l2::map(handle
                                                          , buffer_count)
                                                ));
        }
    }

    ~v4l2_object_t()
    {
        v4l2::unmap(handle
              , mapped_buffer);
        v4l2::close_device(handle);
    }


    bool fetch_frame_format(frame_size_t& frame_size
                            , pixel_format_t& pixel_format)
    {
        return v4l2::fetch_frame_format(handle
                                        , frame_size
                                        , pixel_format);
    }

    bool fetch_fps(std::uint32_t& fps)
    {
        return v4l2::fetch_fps(handle
                               , fps);
    }

    bool fetch_frame_info(frame_info_t& frame_info)
    {
        return fetch_frame_format(frame_info.size
                                  , frame_info.pixel_format)
                && fetch_fps(frame_info.fps);
    }

    format_list_t fetch_supported_format_list()
    {
        return std::move(v4l2::fetch_supported_format(handle));
    }
    bool set_frame_format(const frame_size_t& frame_size
                          , pixel_format_t pixel_format)
    {

        return v4l2::set_frame_format(handle
                                      , frame_size
                                      , pixel_format);
    }

    bool set_fps(std::uint32_t fps)
    {
        return v4l2::set_fps(handle
                             , fps);
    }

    bool set_frame_info(const frame_info_t& frame_info)
    {
        return set_frame_format(frame_info.size
                                , frame_info.pixel_format)
                && set_fps(frame_info.fps);
    }

    control_list_t fetch_control_list()
    {
        return v4l2::fetch_control_list(handle);
    }

    frame_data_t fetch_frame_data(std::uint32_t timeout = 0)
    {
        return v4l2::fetch_frame_data(handle
                                      , mapped_buffer
                                      , timeout);
    }

    bool is_open() const
    {
        return handle >= 0;
    }

};


struct v4l2_device_context_t
{
    stream_data_handler_t               m_stream_data_handler;
    stream_event_handler_t              m_stream_event_handler;
    std::thread                         m_stream_thread;
    std::mutex                          m_mutex;
    format_list_t                       m_format_list;
    frame_info_t                        m_frame_info;
    std::atomic_bool                    m_running;
    std::atomic_bool                    m_established;


    v4l2_device_context_t(stream_data_handler_t stream_data_handler
                          , stream_event_handler_t stream_event_handler)
        : m_stream_data_handler(stream_data_handler)
        , m_stream_event_handler(stream_event_handler)
        , m_running(false)
        , m_established(false)
    {

    }

    bool open(const std::string &uri
              , std::uint32_t buffer_count)
    {
        close();

        m_running = true;
        m_stream_thread = std::thread(&v4l2_device_context_t::stream_proc
                                      , this
                                      , uri
                                      , buffer_count);

        return false;
    }

    bool close()
    {
        if (m_running)
        {
            m_running = false;

            if (m_stream_thread.joinable())
            {
                m_stream_thread.join();
            }

        }
        return false;
    }

    bool is_opened() const
    {
        return m_running;
    }

    bool is_established() const
    {
        return m_established;
    }

    format_list_t get_supported_formats()
    {
        std::lock_guard<std::mutex> lg(m_mutex);
        return m_format_list;
    }

    void stream_proc(std::string uri
                     , std::uint32_t buffer_count)
    {
        push_event(streaming_event_t::start);

        while (m_running)
        {
            std::unique_ptr<v4l2_object_t> device(new v4l2_object_t(uri
                                                                    , m_frame_info
                                                                    , buffer_count));




            if (device->is_open())
            {
                m_frame_info = device->frame_info;

                {
                    auto formats = device->fetch_supported_format_list();

                    std::lock_guard<std::mutex> lg(m_mutex);
                    m_format_list = std::move(formats);
                }

                std::uint32_t frame_time = (1000 / device->frame_info.fps);
                push_event(streaming_event_t::open);

                auto tp = std::chrono::high_resolution_clock::now();

                while(m_running
                      && m_frame_info == device->frame_info)
                {
                    auto frame_data = std::move(device->fetch_frame_data());

                    if (!frame_data.empty())
                    {
                        m_established = true;
                        tp = std::chrono::high_resolution_clock::now();
                        if (m_stream_data_handler != nullptr)
                        {
                            m_stream_data_handler(device->frame_info
                                                  , std::move(frame_data));
                        }
                    }
                    else
                    {
                         auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - tp).count();
                         if (dt < 1000)
                         {
                             std::this_thread::sleep_for(std::chrono::milliseconds(frame_time));
                         }
                         else
                         {
                             break;
                         }
                    }
                }
                //auto device->
                m_established = false;
                push_event(streaming_event_t::close);
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }

        push_event(streaming_event_t::stop);
    }

    void push_event(streaming_event_t capture_event)
    {
        if (m_stream_event_handler != nullptr)
        {
            m_stream_event_handler(capture_event);
        }
    }
};
//------------------------------------------------------------------------------------------
void v4l2_device_context_deleter_t::operator()(v4l2_device_context_t *v4l2_device_context_ptr)
{
    delete v4l2_device_context_ptr;
}
//---------------------------------------------------------------------------------------------
v4l2_device::v4l2_device(stream_data_handler_t stream_data_handler
        , stream_event_handler_t stream_event_handler)
    : m_v4l2_device_capturer_context(new v4l2_device_context_t(stream_data_handler
                                                               , stream_event_handler))
{

}

bool v4l2_device::open(const std::string &uri
                       , std::uint32_t buffer_count)
{
    return m_v4l2_device_capturer_context->open(uri
                                                , buffer_count);
}

bool v4l2_device::close()
{
    return m_v4l2_device_capturer_context->close();
}

bool v4l2_device::is_opened() const
{
    return m_v4l2_device_capturer_context->is_opened();
}

bool v4l2_device::is_established() const
{
    return m_v4l2_device_capturer_context->is_established();
}

format_list_t v4l2_device::get_supported_formats()
{
    return std::move(m_v4l2_device_capturer_context->get_supported_formats());
}

const frame_info_t &v4l2_device::get_format() const
{
    return m_v4l2_device_capturer_context->m_frame_info;
}

bool v4l2_device::set_format(const frame_info_t &format)
{
    m_v4l2_device_capturer_context->m_frame_info = format;
}

bool v4l2_device::set_format(int32_t index)
{
    m_v4l2_device_capturer_context->m_frame_info = m_v4l2_device_capturer_context->m_format_list[index];
}

}
