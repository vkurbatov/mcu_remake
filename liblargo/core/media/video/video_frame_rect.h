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

struct frame_rect_t
{
    frame_point_t   point;
    frame_size_t    size;

    frame_rect_t(const frame_point_t& point = default_frame_point
                 , const frame_size_t& size = default_frame_size);

    frame_rect_t(const frame_point_t& point
                 , const frame_point_t& br_point);

    bool operator == (const frame_rect_t& frame_rect);
    bool operator != (const frame_rect_t& frame_rect);

    frame_rect_t& operator += (const frame_point_t& frame_point);
    frame_rect_t& operator -= (const frame_point_t& frame_point);

    frame_rect_t& operator += (const frame_size_t& frame_size);
    frame_rect_t& operator -= (const frame_size_t& frame_size);

    frame_point_t br_point() const;
};

}

}

}

#endif // VIDEO_FRAME_RECT_H
