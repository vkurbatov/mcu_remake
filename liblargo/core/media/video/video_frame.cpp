#include "video_frame.h"
#include "media/common/media_frame.h"
#include "media/common/media_frame_buffered.h"

namespace core
{

namespace media
{

namespace video
{

video_frame::video_frame(const video_format_t& video_format
                         , i_media_buffer& media_buffer)
    : m_video_format(video_format)
    , m_media_frame_ptr(new media_frame(m_video_format
                        , media_buffer))
{

}

video_frame::video_frame(const video_format_t& video_format
                         , media::media_buffer&& media_buffer)
    : m_video_format(video_format)
    , m_media_frame_ptr(new media_frame_buffered(m_video_format
                                                , std::move(media_buffer)))
{

}

const video_format_t &video_frame::video_format() const
{
    return m_video_format;
}

const media_format_t &video_frame::media_format() const
{
    return m_media_frame_ptr->media_format();
}

media_plane_list_t video_frame::planes() const
{
    return m_media_frame_ptr->planes();
}

std::size_t video_frame::size() const
{
    return m_media_frame_ptr->size();
}

bool video_frame::is_valid() const
{
    return m_media_frame_ptr->is_valid();
}

}

}

}
