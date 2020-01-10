#ifndef VIDEO_FRAME_POINT_H
#define VIDEO_FRAME_POINT_H

#include <cstdint>

namespace core
{

namespace media
{

namespace video
{

const std::uint32_t default_frame_x = 0;
const std::uint32_t default_frame_y = 0;

struct frame_point_t
{
    std::uint32_t   x;
    std::uint32_t   y;

    frame_point_t(std::uint32_t x = default_frame_x
                 , std::uint32_t y = default_frame_y);

    bool operator == (const frame_point_t& frame_size) const;
    bool operator != (const frame_point_t& frame_size) const;
    frame_point_t& operator += (const frame_point_t& frame_point);
    frame_point_t& operator -= (const frame_point_t& frame_point);

    bool is_null() const;
};

const frame_point_t default_frame_point = { default_frame_x
                                        , default_frame_y };

}

}

}

#endif // VIDEO_FRAME_POINT_H
