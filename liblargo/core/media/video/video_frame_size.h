#ifndef VIDEO_FRAME_SIZE_H
#define VIDEO_FRAME_SIZE_H

#include <cstdint>

namespace core
{

namespace media
{

namespace video
{

const std::uint32_t default_frame_width = 0;
const std::uint32_t default_frame_height = 0;

struct frame_size_t
{
    std::uint32_t   width;
    std::uint32_t   height;

    frame_size_t(std::uint32_t width = default_frame_width
                 , std::uint32_t height = default_frame_height);

    bool operator == (const frame_size_t& frame_size) const;
    bool operator != (const frame_size_t& frame_size) const;
    frame_size_t& operator += (const frame_size_t& frame_size);
    frame_size_t& operator -= (const frame_size_t& frame_size);

    bool is_empty() const;
};

const frame_size_t default_frame_size = { default_frame_width
                                        , default_frame_height };

}

}

}

#endif // VIDEO_FRAME_SIZE_H
