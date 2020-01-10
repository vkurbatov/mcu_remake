#include "video_frame_point.h"
namespace core
{

namespace media
{

namespace video
{

frame_point_t::frame_point_t(uint32_t x
                           , uint32_t y)
    : x(x)
    , y(y)
{

}

bool frame_point_t::operator==(const frame_point_t &frame_point) const
{
    return x == frame_point.x
            && y == frame_point.y;
}

bool frame_point_t::operator !=(const frame_point_t &frame_point) const
{
    return ! operator ==(frame_point);
}

frame_point_t &frame_point_t::operator +=(const frame_point_t &frame_point)
{
    x += frame_point.x;
    y += frame_point.y;
    return *this;
}

frame_point_t &frame_point_t::operator -=(const frame_point_t &frame_point)
{
    x -= frame_point.x;
    y -= frame_point.y;
    return *this;
}

bool frame_point_t::is_null() const
{
    return x == 0
            && y == 0;
}

}

}

}
