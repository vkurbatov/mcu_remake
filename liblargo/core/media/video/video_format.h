#ifndef VIDEO_FORMAT_H
#define VIDEO_FORMAT_H

#include "video_frame_size.h"
#include "media/common/media_format.h"

namespace core
{

namespace media
{

namespace video
{

enum class pixel_format_t
{
    unknown,
    yuv420p,
    yuv422p,
    yuv444p,
    yuyv,
    uyvy,
    yuv411p,
    yuv410,
    rgb555,
    rgb555x,
    rgb565,
    rgb565x,
    bgr24,
    rgb24,
    bgr32,
    rgb32,
    abgr32,
    argb32,
    gray8,
    gray16,
    gray16x,
    nv12,
    sbggr8,
    sgbrg8,
    sgrbg8,
    srggb8,
    mjpeg,
    h264,
    h263,
    vp8,
    vp9,
    mpeg4,
    cpia
};

const pixel_format_t default_pixel_format = pixel_format_t::yuv420p;
const std::uint32_t default_fps = 0;


struct video_format_t : public media_format_t
{
    pixel_format_t  pixel_format;
    frame_size_t    size;
    std::uint32_t   fps;

    static bool is_planar(pixel_format_t pixel_format);
    static bool is_encoded(pixel_format_t pixel_format);
    static std::size_t bpp(pixel_format_t pixel_format);
    static std::size_t frame_size(pixel_format_t pixel_format
                                  , const frame_size_t& size);

    static std::size_t planes(pixel_format_t pixel_format);
    static std::size_t plane_width(pixel_format_t pixel_format
                                   , std::uint32_t width
                                   , std::uint32_t plane_idx);
    static std::size_t plane_size(pixel_format_t pixel_format
                                  , const frame_size_t& size
                                  , std::uint32_t plane_idx);

    video_format_t(pixel_format_t pixel_format = default_pixel_format
                   , frame_size_t size = default_frame_size
                   , std::uint32_t fps = default_fps);

    bool operator ==(const video_format_t& video_format);
    bool operator !=(const video_format_t& video_format);

    bool is_planar() const override;
    bool is_encoded() const override;
    std::size_t bpp() const;
    std::size_t frame_size() const override;

    std::size_t planes() const override;
    std::size_t plane_width(std::uint32_t plane_idx) const;
    std::size_t plane_size(std::uint32_t plane_idx) const;
};

}

}

}

#endif // VIDEO_FORMAT_H
