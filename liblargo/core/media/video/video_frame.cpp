#include "video_frame.h"

namespace core
{

namespace media
{

namespace video
{

video_frame::video_frame(const video_format_t &video_format
                         , i_media_buffer &media_buffer)
    : m_video_format(video_format)
    , m_media_frame(m_video_format
                     , media_buffer)
{

}

const media_format_t &video_frame::media_format() const
{
    return m_media_frame.media_format();
}

media_plane_list_t video_frame::planes() const
{
    return m_media_frame.planes();
}

const video_format_t &video_frame::video_format() const
{
    return m_video_format;
}

}

}

}
