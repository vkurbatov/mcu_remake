#ifndef CV_BASE_H
#define CV_BASE_H

#include <string>

namespace opencv
{

enum class font_t
{
    simplex,
    plain,
    duplex,
    complex,
    triplex,
    complex_small,
    script_simplex,
    script_complex,
};

enum class v_align_t
{
    left,
    right,
    center
};

enum class h_align_t
{
    top,
    bottom,
    center
};

typedef std::uint32_t color_t;

struct frame_size_t; //fwd

struct frame_point_t
{
    std::uint32_t   x;
    std::uint32_t   y;

    frame_point_t(std::uint32_t x = 0
                  , std::uint32_t y = 0);

    bool is_null() const;

    bool operator ==(const frame_point_t& frame_point) const;
    bool operator !=(const frame_point_t& frame_point) const;

    frame_point_t& operator +=(const frame_point_t& frame_point);
    frame_point_t& operator +=(const frame_size_t& frame_size);
};

struct frame_size_t
{
    std::uint32_t   width;
    std::uint32_t   height;

    frame_size_t(std::uint32_t width = 0
                  , std::uint32_t height = 0);

    std::size_t size() const;
    bool is_null() const;
    bool operator ==(const frame_size_t& frame_size) const;
    bool operator !=(const frame_size_t& frame_size) const;

    frame_size_t& operator +=(const frame_size_t& frame_size);
    frame_size_t& operator +=(const frame_point_t& frame_point);

};

struct frame_rect_t
{
    frame_point_t  offset;
    frame_size_t   size;

    frame_rect_t(const frame_point_t& offset = { 0, 0 }
            , const frame_size_t& size = { 0, 0 });

    frame_rect_t(std::uint32_t x
                 , std::uint32_t y
                 , std::uint32_t width
                 , std::uint32_t height);

    bool operator ==(const frame_rect_t& frame_rect) const;
    bool operator !=(const frame_rect_t& frame_rect) const;

    frame_rect_t& operator +=(const frame_size_t& frame_size);
    frame_rect_t& operator +=(const frame_point_t& frame_point);

    bool is_join(const frame_size_t& frame_size) const;
    bool is_null() const;
};

struct draw_format_t
{
    color_t         color;
    std::int32_t    thickness;

    draw_format_t(color_t color = 0
                  , std::int32_t thickness = 1);

};

struct text_format_t : public draw_format_t
{
    font_t              font;
    double              scale_factor;
    bool                italic;
    v_align_t           v_align;
    h_align_t           h_align;

    static frame_size_t text_size(const std::string& text
                                  , font_t font
                                  , std::uint32_t scale_factor = 1.0
                                  , bool italic = false
                                  , std::int32_t thickness = 1);

    text_format_t(font_t font = font_t::simplex
                  , std::uint32_t scale_factor = 1.0
                  , bool italic = false
                  , color_t color = 0
                  , std::int32_t thickness = 1
                  , v_align_t v_align = v_align_t::left
                  , h_align_t h_align = h_align_t::top);

    frame_size_t text_size(const std::string& text) const;
};

void draw_text(const std::string& text
               , void* frame_data
               , const frame_size_t& frame_size
               , const text_format_t& text_format
               , const frame_point_t& text_point
               , std::int32_t text_height = 0);

void draw_image(const void* input_frame_data
                , const frame_size_t& input_frame_size
                , const frame_rect_t& input_frame_rect
                , void* output_frame_data
                , const frame_point_t& output_frame_point
                , const frame_size_t& output_frame_size
                , double opacity = 0.0);

void draw_image(const void* input_frame_data
                , void* output_frame_data
                , const frame_size_t& frame_size
                , double opacity = 0.0);

}

#endif // CV_BASE_H
