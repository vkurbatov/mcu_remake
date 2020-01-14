#include "video_frame_buffered.h"

namespace core
{

namespace media
{

namespace video
{

video_frame_buffered::video_frame_buffered(const video_format_t &video_format
                                           , media::media_buffer&& media_buffer)
    : media_frame_buffered(m_video_format = video_format
                           , std::move(media_buffer))
{

}

const video_format_t &video_frame_buffered::video_format() const
{
    return m_video_format;
}

}

}

}
