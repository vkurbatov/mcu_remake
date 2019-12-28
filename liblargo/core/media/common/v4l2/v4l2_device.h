#ifndef V4L2_DEVICE_H
#define V4L2_DEVICE_H

#include "v4l2_base.h"

namespace v4l2
{

struct v4l2_device_context_t;
struct v4l2_device_context_deleter_t { void operator()(v4l2_device_context_t* v4l2_device_context_ptr); };

typedef std::unique_ptr<v4l2_device_context_t, v4l2_device_context_deleter_t> libav_stream_capturer_context_ptr_t;

class v4l2_device
{
    libav_stream_capturer_context_ptr_t m_v4l2_device_capturer_context;
    stream_data_handler_t               m_stream_data_handler;
    stream_event_handler_t              m_stream_event_handler;
public:    
    v4l2_device(stream_data_handler_t stream_data_handler = nullptr
            , stream_event_handler_t stream_event_handler = nullptr);

    bool open(const std::string& uri
              , std::uint32_t buffer_count = 1);
    //          , stream_mask_t stream_mask = stream_mask_t::stream_mask_all);
    bool close();
    bool is_opened() const;
    bool is_established() const;
    format_list_t get_supported_formats();
    const frame_info_t& get_format() const;
    bool set_format(const frame_info_t& format);
    bool set_format(std::int32_t index);
    // media_queue_t fetch_media_queue(std::int32_t stream_id);
};

}


#endif // V4L2_DEVICE_H
