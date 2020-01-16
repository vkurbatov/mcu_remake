#include "video_filter_flip.h"
#include "media/video/i_video_frame.h"
#include "media/video/video_utils.h"

namespace core
{

namespace media
{

namespace video
{

namespace filters
{

video_filter_flip::video_filter_flip(flip_method_t flip_method)
    : m_flip_method(flip_method)
{

}

flip_method_t video_filter_flip::flip_method() const
{
    return m_flip_method;
}

void video_filter_flip::set_flip_method(flip_method_t flip_method)
{
    m_flip_method = flip_method;
}

bool video_filter_flip::internal_filter(i_video_frame &video_frame) const
{
    switch(m_flip_method)
    {
        case flip_method_t::vertical:
            return video_utils::vertical_flip(video_frame);
        break;
        case flip_method_t::horizontal:
            return video_utils::horizontal_flip(video_frame);
        break;
        case flip_method_t::both:
            return video_utils::fast_rotate(video_frame);
        break;
    }

    return false;
}

}

}

}

}
