#ifndef VIDEO_FRAME_RECT_H
#define VIDEO_FRAME_RECT_H

#include "video_frame_point.h"
#include "video_frame_size.h"

namespace core
{

namespace media
{

namespace video
{

template<typename T>
struct frame_rect_base_t
{
    frame_point_base_t<T>   point;
    frame_size_base_t<T>    size;

    frame_rect_base_t(const frame_point_base_t<T> & point = default_frame_point
                 , const frame_size_base_t<T>& size = default_frame_size);

    frame_rect_base_t(const frame_point_base_t<T> & point
                 , const frame_point_base_t<T> & br_point);

    frame_rect_base_t(const frame_size_base_t<T> & size);

    frame_rect_base_t(T x
                     , T y
                     , T width
                     , T height);

    bool operator == (const frame_rect_base_t<T>& frame_rect);
    bool operator != (const frame_rect_base_t<T>& frame_rect);

    frame_rect_base_t<T>& operator += (const frame_point_base_t<T> & frame_point);
    frame_rect_base_t<T>& operator -= (const frame_point_base_t<T> & frame_point);

    frame_rect_base_t<T>& operator += (const frame_size_base_t<T>& frame_size);
    frame_rect_base_t<T>& operator -= (const frame_size_base_t<T>& frame_size);

    frame_point_base_t<T> br_point() const;

    frame_rect_base_t<T>& set_br_point(const frame_point_base_t<T>& frame_point);

    bool is_join(const frame_size_base_t<T>& size) const;
    frame_rect_base_t<T>&  merge(const frame_rect_base_t<T>& frame_rect);
    frame_rect_base_t<T>&  cut(const frame_rect_base_t<T>& frame_rect);
    frame_rect_base_t<T>&  put(const frame_point_base_t<T>& frame_point);
};

typedef frame_rect_base_t<std::int32_t> frame_rect_t;
typedef frame_rect_base_t<double> relative_frame_rect_t;

/*
struct frame_rect_t
{
    frame_point_t   point;
    frame_size_t    size;

    frame_rect_t(const frame_point_t& point = default_frame_point
                 , const frame_size_t& size = default_frame_size);

    frame_rect_t(const frame_point_t& point
                 , const frame_point_t& br_point);

    frame_rect_t(std::int32_t x
                 , std::int32_t y
                 , std::int32_t width
                 , std::int32_t height);

    bool operator == (const frame_rect_t& frame_rect);
    bool operator != (const frame_rect_t& frame_rect);

    frame_rect_t& operator += (const frame_point_t& frame_point);
    frame_rect_t& operator -= (const frame_point_t& frame_point);

    frame_rect_t& operator += (const frame_size_t& frame_size);
    frame_rect_t& operator -= (const frame_size_t& frame_size);

    frame_point_t br_point() const;

    bool is_join(const frame_size_t& size) const;
};
*/
}

}

}

#endif // VIDEO_FRAME_RECT_H
