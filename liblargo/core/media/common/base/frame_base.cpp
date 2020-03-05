#include "frame_base.h"
#include <algorithm>

namespace base
{

frame_point_t::frame_point_t(uint32_t x, uint32_t y)
    : x(x)
    , y(y)
{

}

bool frame_point_t::is_null() const
{
    return x == 0 && y == 0;
}

bool frame_point_t::operator ==(const frame_point_t &frame_point) const
{
    return x == frame_point.x
            && y == frame_point.y;
}

bool frame_point_t::operator !=(const frame_point_t &frame_point) const
{
    return !operator==(frame_point);
}

frame_point_t &frame_point_t::operator +=(const frame_point_t &frame_point)
{
    x += frame_point.x;
    y += frame_point.y;

    return *this;
}

frame_point_t &frame_point_t::operator +=(const frame_size_t &frame_size)
{
    x += frame_size.width;
    y += frame_size.height;

    return *this;
}

frame_size_t::frame_size_t(uint32_t width, uint32_t height)
    : width(width)
    , height(height)
{

}

std::size_t frame_size_t::size() const
{
    return width * height;
}

bool frame_size_t::is_null() const
{
    return width == 0 || height == 0;
}

bool frame_size_t::operator ==(const frame_size_t &frame_size) const
{
    return width == frame_size.width
            && height == frame_size.height;
}

bool frame_size_t::operator !=(const frame_size_t &frame_size) const
{
    return !operator==(frame_size);
}

frame_size_t &frame_size_t::operator +=(const frame_size_t &frame_size)
{
    width += frame_size.width;
    height += frame_size.height;

    return *this;
}

frame_size_t &frame_size_t::operator +=(const frame_point_t &frame_point)
{
    width += frame_point.x;
    height += frame_point.y;

    return *this;
}

frame_rect_t::frame_rect_t(const frame_point_t &offset, const frame_size_t &size)
    : offset(offset)
    , size(size)
{

}

frame_rect_t::frame_rect_t(uint32_t x
                           , uint32_t y
                           , uint32_t width
                           , uint32_t height)
    : frame_rect_t({ x, y }
                   , { width, height })
{

}

void frame_rect_t::aspect_ratio(const frame_rect_t &input_rect
                                , frame_rect_t &output_rect)
{
    auto adjusted_width = (output_rect.size.height * input_rect.size.width) / input_rect.size.height;
    auto adjusted_height = (output_rect.size.width * input_rect.size.height) / input_rect.size.width;

    auto new_width = std::min(adjusted_width, output_rect.size.width);
    auto new_height = std::min(adjusted_height, output_rect.size.height);

    output_rect.offset.x += (output_rect.size.width - new_width) / 2;
    output_rect.offset.y += (output_rect.size.height - new_height) / 2;

    output_rect.size.width = new_width;
    output_rect.size.height = new_height;
}

bool frame_rect_t::operator ==(const frame_rect_t &frame_rect) const
{
    return offset == frame_rect.offset && size == frame_rect.size;
}

bool frame_rect_t::operator !=(const frame_rect_t &frame_rect) const
{
    return !operator==(frame_rect);
}

frame_rect_t &frame_rect_t::operator +=(const frame_size_t &frame_size)
{
    size += frame_size;
    return *this;
}

frame_rect_t &frame_rect_t::operator +=(const frame_point_t &frame_point)
{
    offset += frame_point;
    return *this;
}

bool frame_rect_t::is_join(const frame_size_t &frame_size) const
{
    return frame_size.width >= (offset.x + size.width)
            && frame_size.height >= (offset.y + size.height);
}

void frame_rect_t::aspect_ratio(const frame_rect_t &rect)
{
    aspect_ratio(rect
                 , *this);
}

bool frame_rect_t::is_null() const
{
    return offset.is_null()
            && size.width == 0 && size.height == 0;
}

}
