#include "video_frame_rect.h"


namespace core
{

namespace media
{

namespace video
{

frame_rect_t::frame_rect_t(const frame_point_t &point
                           , const frame_size_t &size)
    : point(point)
    , size(size)
{

}

frame_rect_t::frame_rect_t(const frame_point_t &point
                           , const frame_point_t &br_point)
    : point(point)
    , size(br_point.x - point.x
           , br_point.y - point.y)
{

}

bool frame_rect_t::operator ==(const frame_rect_t &frame_rect)
{
    return point == frame_rect.point
            && size == frame_rect.size;

}

bool frame_rect_t::operator !=(const frame_rect_t &frame_rect)
{
    return !operator ==(frame_rect);
}

frame_rect_t& frame_rect_t::operator +=(const frame_point_t &frame_point)
{
    point += frame_point;
    return *this;
}

frame_rect_t& frame_rect_t::operator -=(const frame_point_t &frame_point)
{
    point -= frame_point;
    return *this;
}

frame_rect_t& frame_rect_t::operator +=(const frame_size_t &frame_size)
{
    size += frame_size;
    return *this;
}

frame_rect_t& frame_rect_t::operator -=(const frame_size_t &frame_size)
{
    size -= frame_size;
    return *this;
}

frame_point_t frame_rect_t::br_point() const
{
    return frame_point_t(point.x + size.width, point.y + size.height);
}



}

}

}
