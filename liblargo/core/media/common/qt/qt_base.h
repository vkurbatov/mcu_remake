#ifndef QT_BASE_H
#define QT_BASE_H

#include "media/common/base/frame_base.h"

#include <cstdint>
#include <string>
#include <vector>

namespace qt
{

using frame_point_t = base::frame_point_t;
using frame_size_t = base::frame_size_t;
using frame_rect_t = base::frame_rect_t;


enum class h_align_t
{
    left,
    right,
    center
};

enum class v_align_t
{
    top,
    bottom,
    center
};

enum class pixel_format_t
{
    rgb24 = 13,
    rgb32 = 4,
    argb32 = 5,
    rgba32 = 17,
};

const auto default_pixel_format = pixel_format_t::rgb24;

typedef std::uint32_t color_t;
typedef std::vector<frame_point_t> polylines_t;

struct draw_format_t
{
    color_t         border_color;
    std::int32_t    border_weight;
    color_t         background_color;

    draw_format_t(color_t border_color = 0x00000000
                  , std::int32_t border_weight = 1
                  , color_t background_color = 0x000000FF);

};

struct font_t
{
    std::string     name;
    std::int32_t    size;
    std::int32_t    weight;
    bool            italic;

    static frame_size_t text_size(const std::string& text
                                  , const std::string& name
                                  , std::int32_t size = -1
                                  , std::int32_t weight = -1
                                  , bool italic = false);

    font_t(const std::string& name = ""
           , std::int32_t size = -1
           , std::int32_t weight = -1
           , bool italic = false);

    frame_size_t text_size(const std::string& text) const;
};

const auto font_default = font_t();

struct text_format_t
{
    font_t          font;
    color_t         color;

    text_format_t(font_t font = font_default
                  , color_t color = 0x00000000);

};

void draw_text(const std::string& text
               , const text_format_t& text_format
               , const frame_point_t& text_point
               , void *frame_data
               , const frame_size_t& frame_size
               , pixel_format_t pixel_format = default_pixel_format);


void draw_text(const std::string& text
               , const text_format_t& text_format
               , const frame_rect_t& text_rect
               , h_align_t h_align
               , v_align_t v_align
               , void *frame_data
               , const frame_size_t& frame_size
               , pixel_format_t pixel_format = default_pixel_format);

void draw_rect(const draw_format_t& draw_format
               , const frame_rect_t& draw_rect
               , void *frame_data
               , const frame_size_t& frame_size
               , pixel_format_t pixel_format = default_pixel_format);

void draw_ellipse(const draw_format_t& draw_format
                 , const frame_rect_t& draw_rect
                 , void *frame_data
                 , const frame_size_t& frame_size
                 , pixel_format_t pixel_format = default_pixel_format);

void draw_polygon(const draw_format_t& draw_format
                  , const polylines_t& polylines
                  , void *frame_data
                  , const frame_size_t& frame_size
                  , pixel_format_t pixel_format = default_pixel_format);

void draw_image(const void* input_frame_data
                , const frame_size_t& input_frame_size
                , const frame_rect_t& input_frame_rect
                , void* output_frame_data
                , const frame_size_t& output_frame_size
                , const frame_rect_t& output_frame_rect
                , double opacity = 0.0
                , pixel_format_t pixel_format = default_pixel_format);

void draw_image(const void* input_frame_data
                , const frame_size_t& input_frame_size
                , void* output_frame_data
                , const frame_size_t& output_frame_size
                , double opacity = 0.0
                , pixel_format_t pixel_format = default_pixel_format);
}

#endif // QT_BASE_H
