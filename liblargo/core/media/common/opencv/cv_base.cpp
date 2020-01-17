#include "cv_base.h"

#include <opencv2/imgproc.hpp>

namespace opencv
{

const auto opencv_image_type = CV_MAKETYPE(CV_8U, 3);

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
               , const frame_size_t &frame_size
               , double font_scale
               , std::int32_t pixel_width)
{
    cv::Mat matrix(frame_size.height
              , frame_size.width
              , CV_MAKETYPE(CV_8U, pixel_width)
              , frame_data);

    auto font = cv::FONT_HERSHEY_PLAIN;
    auto thickness = 2;

    auto sz = cv::getTextSize(text.c_str()
                    , font
                    , 1.0
                    , thickness
                    , nullptr);

    font_scale = (frame_size.height * font_scale) / (sz.height);

    cv::Rect text_rect((frame_size.width - sz.width * font_scale)/ 2
                       , (frame_size.height - sz.height * font_scale) / 2
                       , sz.width * font_scale
                       , sz.height * font_scale);


    cv::Scalar color(0, 255, 0);

    auto roi = matrix(text_rect);

    text_rect.x = 0;
    text_rect.y = 0;

    cv::Mat overlay(roi.clone());


    // cv::addWeighted(overlay, opacity, img, 1 - opacity, 0, img)

    cv::rectangle(overlay
                  , text_rect
                  , color
                  , thickness);

    cv::putText(overlay
              , text.c_str()
              , cv::Point(text_rect.x
                          , text_rect.y + text_rect.height) //top-left position
              , font
              , font_scale
              , color
              , thickness);

    double opacity = 0.1;
    cv::addWeighted(overlay, opacity, roi, 1.0 - opacity, 0.0, roi);
}

draw_format_t::draw_format_t(color_t color
                             , int32_t thickness)
    : color(color)
    , thickness(thickness)
{

}

frame_size_t text_format_t::text_size(const std::string &text
                                      , font_t font
                                      , uint32_t scale_factor
                                      , bool italic
                                      , std::int32_t thickness)
{
    auto size = cv::getTextSize(text.c_str()
                             , static_cast<std::int32_t>(font) | italic ? cv::FONT_ITALIC : 0
                             , scale_factor
                             , thickness
                             , nullptr);

    return { size.width, size.height };
}

text_format_t::text_format_t(font_t font
                             , uint32_t scale_factor
                             , bool italic
                             , color_t color
                             , int32_t thickness
                             , v_align_t v_align
                             , h_align_t h_align)
    : draw_format_t(color
                    , thickness)
    , font(font)
    , scale_factor(scale_factor)
    , italic(italic)
    , v_align(v_align)
    , h_align(h_align)
{

}

frame_size_t text_format_t::text_size(const std::string &text) const
{
    return text_size(text
                     , font
                     , scale_factor
                     , italic
                     , thickness);
}

void draw_text(const std::string &text
               , void *frame_data
               , const frame_size_t& frame_size
               , const text_format_t &text_format
               , const frame_point_t &text_point
               , int32_t text_height)
{
    cv::Mat matrix(frame_size.height
                  , frame_size.width
                  , opencv_image_type
                  , frame_data);

    auto scale_factor = text_format.scale_factor;

    std::int32_t dx = 0;
    std::int32_t dy = 0;

    auto text_size = text_format.text_size(text);

    switch(text_format.v_align)
    {
        case v_align_t::left:
            dx = 0;
        break;
        case v_align_t::right:
            dx = text_size.width - 1;
        break;
        case v_align_t::center:
            dx = (text_size.width - 1) / 2;
        break;
    }

    switch(text_format.h_align)
    {
        case h_align_t::top:
            dy = text_size.height - 1;
        break;
        case h_align_t::bottom:
            dy = 0;
        break;
        case h_align_t::center:
            dy = (text_size.height - 1) / 2;
        break;
    }

    if (text_height != 0)
    {
        scale_factor = static_cast<double>(text_height)
                / static_cast<double>(text_size.height);

    }

    cv::putText(matrix
              , text.c_str()
              , cv::Point((text_point.x - dx * scale_factor)
                          , (text_point.y + dy * scale_factor))

              , static_cast<std::int32_t>(text_format.font) | text_format.italic ? cv::FONT_ITALIC : 0
              , scale_factor * text_format.scale_factor
              , cv::Scalar((text_format.color) & 0xFF
                           , (text_format.color >> 8) & 0xFF
                           , (text_format.color >> 16) & 0xFF)
              , text_format.thickness);

    cv::circle(matrix
             , { text_point.x, text_point.y }
             , 1
             , cv::Scalar(255, 0, 0)
             , 2);

}

/*
void draw_image(const void *input_frame_data
                , void *output_frame_data
                , const frame_rect_t &input_frame_rect
                , const frame_rect_t &output_frame_rect
                , double opacity)
{
    cv::Mat input_matrix(input_frame_data);
}*/

void draw_image(const void *input_frame_data
                , const frame_size_t &input_frame_size
                , const frame_rect_t &input_frame_rect
                , void *output_frame_data
                , const frame_point_t &output_frame_point
                , const frame_size_t &output_frame_size
                , double opacity)
{
    auto input_rect = cv::Rect(input_frame_rect.offset.x
                                    , input_frame_rect.offset.y
                                    , input_frame_rect.size.width
                                    , input_frame_rect.size.height);

    auto output_rect = cv::Rect(output_frame_point.x
                                    , output_frame_point.y
                                    , input_rect.width
                                    , input_rect.height);

    cv::Mat input_matrix(cv::Mat(input_frame_size.height
                         , input_frame_size.width
                         , opencv_image_type
                         , const_cast<void *>(input_frame_data))(input_rect));

    cv::Mat output_matrix(cv::Mat(output_frame_size.height
                         , output_frame_size.width
                         , opencv_image_type
                         , output_frame_data)(output_rect));

    if (opacity == 0.0)
    {
        input_matrix.copyTo(output_matrix);
    }
    else
    {
        cv::addWeighted(input_matrix, opacity, output_matrix, 1.0 - opacity, 0.0, output_matrix);
    }

}

void draw_image(const void *input_frame_data
                , void *output_frame_data
                , const frame_size_t &frame_size
                , double opacity)
{
    draw_image(input_frame_data
               , frame_size
               , { 0, 0 , frame_size.width, frame_size.height }
               , output_frame_data
               , { 0, 0 }
               , frame_size
               , opacity);
}


}
