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
    bgr555,
    bgr555x,
    bgr565,
    bgr565x,
    rgb555,
    rgb555x,
    rgb565,
    rgb565x,
    bgr8,
    rgb8,
    bgr24,
    rgb24,
    bgr32,
    rgb32,
    abgr32,
    argb32,
    bgra32,
    rgba32,
    gray8,
    gray16,
    gray16x,
    nv12,
    sbggr8,
    sgbrg8,
    sgrbg8,
    srggb8,
    jpeg,
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

struct video_info_t : public i_format_info
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
    static plane_sizes_t plane_sizes(pixel_format_t pixel_format
                                     , const frame_size_t& size);
    static frame_size_t plane_size(pixel_format_t pixel_format
                                   , const frame_size_t& size
                                   , std::uint32_t plane_idx);

    static std::string to_string(pixel_format_t pixel_format
                                , frame_size_t size
                                , std::uint32_t fps);

    video_info_t(pixel_format_t pixel_format = default_pixel_format
                   , frame_size_t size = default_frame_size
                   , std::uint32_t fps = default_fps);

    bool operator ==(const video_info_t& video_info);
    bool operator !=(const video_info_t& video_info);

    bool is_planar() const override;
    bool is_encoded() const override;
    bool is_convertable() const override;
    std::size_t bpp() const;
    std::size_t frame_size() const override;
    std::size_t planes() const override;
    plane_sizes_t plane_sizes() const override;

    frame_size_t plane_size(std::uint32_t plane_idx) const;

    std::string to_string() const;
};

}

}

}

#endif // VIDEO_FORMAT_H
