#include "v4l2_device.h"
#include "v4l2_api.h"

#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <map>
#include <future>

#define WBS_MODULE_NAME "v4l2:device"
#include <core-tools/logging.h>

namespace v4l2
{

const std::size_t max_frame_queue = 10;

struct command_controller_t
{

    struct request_t
    {
        std::uint32_t               control_id;
        std::int32_t                value;
        bool                        is_set;

        std::promise<std::int32_t>  promise;

        request_t(std::uint32_t control_id
                  , std::int32_t value)
            : control_id(control_id)
            , value(value)
            , is_set(true)
        {

        }

        request_t(std::uint32_t control_id)
            : control_id(control_id)
            , value(0)
            , is_set(false)
        {

        }

        void set_result(std::int32_t value = 0)
        {
            promise.set_value(value);
        }

        bool get_result(std::uint32_t& value
                        , std::uint32_t timeout)
        {
            auto future = promise.get_future();

            if (future.wait_for(std::chrono::milliseconds(timeout)) == std::future_status::ready)
            {
                value = future.get();
                return true;
            }

            return false;
        }
    };

    typedef std::queue<request_t> request_queue_t;

    std::atomic_bool            is_request;
    std::mutex                  mutex;
    request_queue_t             request_queue;



    command_controller_t()
        : is_request(false)
    {

    }

    bool set_control(std::uint32_t control_id
                     , std::int32_t value
                     , std::uint32_t timeout)
    {
        request_t request(control_id
                          , value);

        auto future = request.promise.get_future();

        {
            std::lock_guard<std::mutex> lg(mutex);
            request_queue.emplace(std::move(request));
            is_request = true;
        }

        return future.wait_for(std::chrono::milliseconds(timeout)) == std::future_status::ready;
    }

    bool get_control(std::uint32_t control_id
                     , std::int32_t& value
                     , std::uint32_t timeout)
    {
        request_t request(control_id);

        auto future = request.promise.get_future();

        {
            std::lock_guard<std::mutex> lg(mutex);
            request_queue.emplace(std::move(request));
            is_request = true;
        }

        auto result = future.wait_for(std::chrono::milliseconds(timeout)) == std::future_status::ready;

        if (result)
        {
            value = future.get();
        }

        return result;
    }

    request_queue_t fetch_request_queue()
    {
        if (is_request)
        {
            std::lock_guard<std::mutex> lg(mutex);
            is_request = false;
            return std::move(request_queue);
        }

        return request_queue_t();
    }
};

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
    command_controller_t                m_command_controller;

    std::atomic_bool                    m_running;
    std::atomic_bool                    m_established;
    std::unique_ptr<v4l2_object_t>      m_device;


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

    bool open_device(std::string uri
                     , std::uint32_t buffer_count
                     , frame_info_t& frame_info)
    {
        if (uri.find("v4l2://") == 0)
        {
            uri = uri.substr(6);
        }

        std::lock_guard<std::mutex> lg(m_mutex);
        m_device.reset();
        m_device.reset(new v4l2_object_t(uri
                                     , frame_info
                                     , buffer_count));


        if (m_device->fetch_frame_info(frame_info))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            auto formats = m_device->fetch_supported_format_list();
            auto controls = m_device->fetch_control_list();

            // formats.push_back(frame_info_t({ 800, 600 }, 15, pixel_format_h264));
            m_format_list = std::move(formats);
            m_control_list = std::move(controls);


            return true;
        }

        return false;
    }


    void stream_proc(std::string uri
                     , std::uint32_t buffer_count)
    {
        push_event(streaming_event_t::start);

        while (m_running)
        {
            std::uint32_t frame_time = 50;


            if (open_device(uri
                             , buffer_count
                             , m_frame_info))
            {
                frame_info_t frame_info = m_frame_info;
                frame_time = frame_info.fps == 0 ? 100 : (1000 / frame_info.fps);
                push_event(streaming_event_t::open);

                auto tp = std::chrono::high_resolution_clock::now();

                while(m_running
                      && m_frame_info == frame_info)
                {        
                    command_process(*m_device);
                    auto frame_data = std::move(m_device->fetch_frame_data(frame_time * 2));

                    if (!frame_data.empty())
                    {
                        m_established = true;
                        tp = std::chrono::high_resolution_clock::now();
                        if (m_stream_data_handler == nullptr
                                || m_stream_data_handler(frame_info
                                                         , std::move(frame_data)) == false)
                        {                       
                            push_media_queue(frame_t(frame_info, std::move(frame_data)));
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

                m_established = false;
                push_event(streaming_event_t::close);
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(frame_time));
            }
        }

        {
            std::lock_guard<std::mutex> lg(m_mutex);
            m_device.reset();
        }
        push_event(streaming_event_t::stop);
    }

    void command_process(v4l2_object_t& v4l2_object)
    {
        auto requests = m_command_controller.fetch_request_queue();

        while(!requests.empty())
        {
            auto& request = requests.front();

            if (request.is_set)
            {
                v4l2_object.set_control(request.control_id
                                        , request.value);
                request.set_result();
            }
            else
            {
                v4l2_object.get_control(request.control_id
                                        , request.value);

                request.set_result(request.value);
            }

            requests.pop();
        }
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

    void push_media_queue(frame_t&& frame)
    {
        std::lock_guard<std::mutex> lg(m_mutex);
        m_frame_queue.emplace(std::move(frame));

        while (m_frame_queue.size() > max_frame_queue)
        {
            m_frame_queue.pop();
        }
    }

    bool set_control(std::uint32_t control_id, std::int32_t value)
    {

        return m_command_controller.set_control(control_id
                                                , value
                                                , 100);

/*
        auto l_tp = std::chrono::high_resolution_clock::now();

        LOG_D << "Set before lock" LOG_END;

        std::lock_guard<std::mutex> lg(m_mutex);

        auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - l_tp).count();

        LOG_D << "Set after unlock = " << dt LOG_END;

        if (m_device != nullptr)
        {
            if (m_device->set_control(control_id, value))
            {
                auto it = m_control_list.find(control_id);

                if (it != m_control_list.end())
                {
                    it->second.current_value = value;
                }

                return true;
            }
        }

        return false;*/
    }

    std::int32_t get_control(std::uint32_t control_id, std::int32_t default_value)
    {
        m_command_controller.get_control(control_id
                                         , default_value
                                         , 100);

        return default_value;
        /*
        std::lock_guard<std::mutex> lg(m_mutex);

        if (m_device != nullptr)
        {
            if (m_device->get_control(control_id, default_value))
            {
                auto it = m_control_list.find(control_id);

                if (it != m_control_list.end())
                {
                    it->second.current_value = default_value;
                }
            }
        }

        return default_value;*/
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
    : m_v4l2_device_context(new v4l2_device_context_t(stream_data_handler
                                                               , stream_event_handler))
{

}

bool v4l2_device::open(const std::string &uri
                       , std::uint32_t buffer_count)
{
    return m_v4l2_device_context->open(uri
                                                , buffer_count);
}

bool v4l2_device::close()
{
    return m_v4l2_device_context->close();
}

bool v4l2_device::is_opened() const
{
    return m_v4l2_device_context->is_opened();
}

bool v4l2_device::is_established() const
{
    return m_v4l2_device_context->is_established();
}

format_list_t v4l2_device::get_supported_formats() const
{
    return std::move(m_v4l2_device_context->get_supported_formats());
}

const frame_info_t &v4l2_device::get_format() const
{
    return m_v4l2_device_context->m_frame_info;
}

bool v4l2_device::set_format(const frame_info_t &format)
{
    m_v4l2_device_context->m_frame_info = format;
}

control_list_t v4l2_device::get_control_list() const
{
    return std::move(m_v4l2_device_context->get_control_list());
}

bool v4l2_device::set_control(uint32_t control_id
                              , int32_t value)
{
    return m_v4l2_device_context->set_control(control_id
                                              , value);
}

int32_t v4l2_device::get_control(uint32_t control_id
                                 , int32_t default_value)
{
    return m_v4l2_device_context->get_control(control_id
                                              , default_value);
}

frame_queue_t v4l2_device::fetch_media_queue()
{
    return std::move(m_v4l2_device_context->fetch_media_queue());
}

}
