#ifndef CV_BASE_H
#define CV_BASE_H

#include <string>

namespace opencv
{

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

void draw_text(const std::string& text
               , void* frame_data
               , const frame_size_t& frame_size
               , std::int32_t pixel_width = 3);

}

#endif // CV_BASE_H
