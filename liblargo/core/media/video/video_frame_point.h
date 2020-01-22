#ifndef VIDEO_FRAME_POINT_H
#define VIDEO_FRAME_POINT_H

#include <cstdint>

namespace core
{

namespace media
{

namespace video
{

const double default_frame_x = 0;
const double default_frame_y = 0;

template<typename T>
struct frame_point_base_t
{
    T   x;
    T   y;

    frame_point_base_t(T x = static_cast<T>(default_frame_x)
                 , T y = static_cast<T>(default_frame_y));

    bool operator == (const frame_point_base_t<T>& frame_size) const;
    bool operator != (const frame_point_base_t<T>& frame_size) const;
    frame_point_base_t<T>& operator += (const frame_point_base_t<T>& frame_point);
    frame_point_base_t<T>& operator -= (const frame_point_base_t<T>& frame_point);

    bool is_null() const;

    frame_point_base_t<T>& merge_min(const frame_point_base_t<T>& frame_point);
    frame_point_base_t<T>& merge_max(const frame_point_base_t<T>& frame_point);
};

typedef frame_point_base_t<std::int32_t> frame_point_t;
typedef frame_point_base_t<double> relative_frame_point_t;

const frame_point_t default_frame_point = { default_frame_x
                                        , default_frame_y };

}

}

}

#endif // VIDEO_FRAME_POINT_H
