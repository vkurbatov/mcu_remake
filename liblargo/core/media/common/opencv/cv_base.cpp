#include "cv_base.h"

#include <opencv/cv.h>
#include <opencv2/imgproc.hpp>
//#include <opencv/highgui.h>

namespace opencv
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

bool frame_rect_t::is_null() const
{
    return offset.is_null()
            && size.width == 0 && size.height == 0;
}

void draw_text(const std::string &text
               , void *frame_data
               , const frame_size_t &frame_size)
{
    cv::Mat matrix(frame_size.width
              , frame_size.height
              , CV_8UC3
              , frame_data
              , frame_size.width * 3);

    cv::putText(matrix
              , text.c_str()
              , cv::Point(frame_size.width / 2, frame_size.height / 2) //top-left position
              , cv::FONT_HERSHEY_DUPLEX
              , 1.0
              , CV_RGB(118, 185, 0) //font color
              , 2);
}

}
