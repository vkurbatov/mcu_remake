#include "video_frame.h"
#include "media/common/media_frame.h"
#include "video_utils.h"

namespace core
{

namespace media
{

namespace video
{

media_frame_ptr_t video_frame::create(const media_format_t &media_format
                                      , media_buffer_ptr_t media_buffer)
{
    media_frame_ptr_t frame;

    if (media_buffer != nullptr && media_format.media_type == media_type_t::video)
    {
        frame.reset(new video_frame(static_cast<const video_format_t&>(media_format)
                                    , media_buffer));
    }

    return frame;
}

video_frame::video_frame(const video_format_t &video_format
                         , media_buffer_ptr_t media_buffer)
    : media_frame(media_buffer)
    , m_video_format(video_format)
{

}

const media_format_t &video_frame::media_format() const
{
    return m_video_format;
}

void video_frame::clear()
{
    video_utils::blackout(*this);
}

const video_format_t &video_frame::video_format() const
{
    return m_video_format;
}

}

}

}
