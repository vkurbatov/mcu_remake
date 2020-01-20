#ifndef VIDEO_FRAME_SIZE_H
#define VIDEO_FRAME_SIZE_H

#include <cstdint>

namespace core
{

namespace media
{

namespace video
{

const double default_frame_width = 0;
const double default_frame_height = 0;

template<typename T>
struct frame_size_base_t
{
    T   width;
    T   height;

    frame_size_base_t(T width = static_cast<T>(default_frame_width)
                 , T height = static_cast<T>(default_frame_height));

    bool operator == (const frame_size_base_t<T>& frame_size) const;
    bool operator != (const frame_size_base_t<T>& frame_size) const;
    frame_size_base_t<T>& operator += (const frame_size_base_t<T>& frame_size);
    frame_size_base_t<T>& operator -= (const frame_size_base_t<T>& frame_size);

    bool is_empty() const;
    std::size_t size() const;
};

/*
struct frame_size_t
{
    std::int32_t   width;
    std::int32_t   height;

    frame_size_t(std::int32_t width = default_frame_width
                 , std::int32_t height = default_frame_height);

    bool operator == (const frame_size_t& frame_size) const;
    bool operator != (const frame_size_t& frame_size) const;
    frame_size_t& operator += (const frame_size_t& frame_size);
    frame_size_t& operator -= (const frame_size_t& frame_size);

    bool is_empty() const;
    std::size_t size() const;
};
*/
typedef frame_size_base_t<std::int32_t> frame_size_t;
typedef frame_size_base_t<double> relative_frame_size_t;

const frame_size_t default_frame_size = { default_frame_width
                                        , default_frame_height };

}

}

}

#endif // VIDEO_FRAME_SIZE_H
