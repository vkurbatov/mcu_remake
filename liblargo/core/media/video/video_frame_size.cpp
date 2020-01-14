#include "video_frame_size.h"
namespace core
{

namespace media
{

namespace video
{

frame_size_t::frame_size_t(int32_t width
                           , int32_t height)
    : width(width)
    , height(height)
{

}

bool frame_size_t::operator==(const frame_size_t &frame_size) const
{
    return width == frame_size.width
            && height == frame_size.height;
}

bool frame_size_t::operator !=(const frame_size_t &frame_size) const
{
    return ! operator ==(frame_size);
}

frame_size_t &frame_size_t::operator +=(const frame_size_t &frame_size)
{
    width += frame_size.width;
    height += frame_size.height;
    return *this;
}

frame_size_t &frame_size_t::operator -=(const frame_size_t &frame_size)
{
    width -= frame_size.width;
    height -= frame_size.height;
    return *this;
}

bool frame_size_t::is_empty() const
{
    return width == 0
            || height == 0;
}

std::size_t frame_size_t::size() const
{
    return width * height;
}

}

}

}
