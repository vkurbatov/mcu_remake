#include "v4l2_device.h"
#include "v4l2_api.h"

#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <map>


namespace v4l2
{

const std::size_t max_frame_queue = 10;

struct v4l2_object_t
{
    std::int32_t handle;
    mapped_buffer_t mapped_buffer;

    v4l2_object_t(const std::string& uri
                  , const frame_info_t& frame_info = frame_info_t()
                  , std::size_t buffer_count = 1)

        : handle(v4l2::open_device(uri))
    {
        if (handle >= 0)
        {
            if (!frame_info.is_null())
            {
                set_frame_info(frame_info);
            }

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

    control_map_t fetch_control_list()
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

    bool set_control(std::uint32_t control_id, std::int32_t value)
    {
        return v4l2::set_control(handle, control_id, value);
    }

    bool get_control(std::uint32_t control_id, std::int32_t& value)
    {
        return v4l2::get_control(handle, control_id, value);
    }

};


struct v4l2_device_context_t
{    
    typedef std::queue<std::pair<std::uint32_t, std::int32_t>> control_queue_t;

    stream_data_handler_t               m_stream_data_handler;
    stream_event_handler_t              m_stream_event_handler;

    std::thread                         m_stream_thread;
    mutable std::mutex                  m_mutex;

    format_list_t                       m_format_list;
    control_map_t                       m_control_list;
    frame_info_t                        m_frame_info;
    frame_queue_t                       m_frame_queue;

    control_queue_t                     m_control_queue;

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

    format_list_t get_supported_formats() const
    {
        std::lock_guard<std::mutex> lg(m_mutex);
        return m_format_list;
    }

    control_list_t get_control_list() const
    {
        std::lock_guard<std::mutex> lg(m_mutex);

        control_list_t control_list;

        for (const auto& c: m_control_list)
        {
            control_list.push_back(c.second);
        }

        return std::move(control_list);
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


            frame_info_t frame_info;
            if (device->fetch_frame_info(frame_info))
            {
                m_frame_info = frame_info;

                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    auto formats = device->fetch_supported_format_list();
                    auto controls = device->fetch_control_list();

                    std::lock_guard<std::mutex> lg(m_mutex);
                    m_format_list = std::move(formats);
                    m_control_list = std::move(controls);

                }

                std::uint32_t frame_time = frame_info.fps == 0 ? 100 : (1000 / frame_info.fps);
                push_event(streaming_event_t::open);

                auto tp = std::chrono::high_resolution_clock::now();

                while(m_running
                      && m_frame_info == frame_info)
                {

                    if (!m_control_queue.empty())
                    {
                        std::lock_guard<std::mutex> lg(m_mutex);
                        while(!m_control_queue.empty())
                        {
                            auto it = m_control_list.find(m_control_queue.front().first);

                            if (it != m_control_list.end())
                            {
                                if (device->set_control(m_control_queue.front().first
                                                    , m_control_queue.front().second))
                                {
                                    it->second.current_value = m_control_queue.front().second;
                                }
                            }

                            m_control_queue.pop();
                            //m_control_queue
                        }
                    }

                    auto frame_data = std::move(device->fetch_frame_data(frame_time));

                    if (!frame_data.empty())
                    {
                        m_established = true;
                        tp = std::chrono::high_resolution_clock::now();
                        if (m_stream_data_handler == nullptr
                                || m_stream_data_handler(frame_info
                                                         , std::move(frame_data)) == false)
                        {
                            std::lock_guard<std::mutex> lg(m_mutex);
                            m_frame_queue.emplace(frame_info, std::move(frame_data));

                            while (m_frame_queue.size() > max_frame_queue)
                            {
                                m_frame_queue.pop();
                            }
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

    frame_queue_t fetch_media_queue()
    {
        std::lock_guard<std::mutex> lg(m_mutex);
        return std::move(m_frame_queue);
    }

    void set_control(std::uint32_t control_id, std::int32_t value)
    {
        std::lock_guard<std::mutex> lg(m_mutex);
        m_control_queue.emplace(control_id, value);

        while(!m_control_queue.empty() > 10)
        {
            m_control_queue.pop();
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

format_list_t v4l2_device::get_supported_formats() const
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

control_list_t v4l2_device::get_control_list() const
{
    return std::move(m_v4l2_device_capturer_context->get_control_list());
}

void v4l2_device::control(uint32_t control_id, int32_t value)
{
    m_v4l2_device_capturer_context->set_control(control_id, value);
}

frame_queue_t v4l2_device::fetch_media_queue()
{
    return std::move(m_v4l2_device_capturer_context->fetch_media_queue());
}

}
