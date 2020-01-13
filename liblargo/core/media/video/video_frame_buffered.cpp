#include "video_frame_buffered.h"

namespace core
{

namespace media
{

namespace video
{

video_frame_buffered::video_frame_buffered(const video_format_t &video_format
                                           , media::media_buffer&& media_buffer)
    : video_frame(video_format
                  , m_media_buffer)
    , m_media_buffer(std::move(media_buffer))
{

}

}

}

}
