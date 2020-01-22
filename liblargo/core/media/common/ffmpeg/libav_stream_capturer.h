#ifndef FFMPEG_LIBAV_STREAM_CAPTURER_H
#define FFMPEG_LIBAV_STREAM_CAPTURER_H

#include "libav_base.h"

namespace ffmpeg
{

struct libav_stream_capturer_context_t;
struct libav_stream_capturer_context_deleter_t { void operator()(libav_stream_capturer_context_t* libav_stream_capturer_context_ptr); };

typedef std::unique_ptr<libav_stream_capturer_context_t, libav_stream_capturer_context_deleter_t> libav_stream_capturer_context_ptr_t;

class libav_stream_capturer
{
    libav_stream_capturer_context_ptr_t m_libav_stream_capturer_context;
    stream_data_handler_t               m_stream_data_handler;
    stream_event_handler_t              m_stream_event_handler;


public:
    libav_stream_capturer(stream_data_handler_t stream_data_handler = nullptr
            , stream_event_handler_t stream_event_handler = nullptr);
    bool open(const std::string& uri
              , stream_mask_t stream_mask = stream_mask_t::stream_mask_all);
    bool close();
    bool is_opened() const;
    bool is_established() const;
    stream_info_list_t streams() const;
    frame_queue_t fetch_media_queue(std::int32_t stream_id);
};

}

#endif // ffmpeg_LIBAV_STREAM_CAPTURER_H
