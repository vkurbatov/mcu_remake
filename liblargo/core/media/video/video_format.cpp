#include "video_format.h"
#include "media/common/utils/format_converter.h"
#include "media/common/ffmpeg/libav_base.h"


namespace core
{

namespace media
{

namespace video
{

//-----------------------------------------------------------------------------------------------------------

bool video_format_t::is_planar(pixel_format_t pixel_format)
{
    return pixel_format < pixel_format_t::rgb555;
}

bool video_format_t::is_encoded(pixel_format_t pixel_format)
{
    return pixel_format >= pixel_format_t::h264;
}

std::size_t video_format_t::bpp(pixel_format_t pixel_format)
{
    return ffmpeg::video_info_t::bpp(utils::format_conversion::to_ffmpeg_format(pixel_format));
}

std::size_t video_format_t::frame_size(pixel_format_t pixel_format
                                       , const frame_size_t &size)
{
    return ffmpeg::video_info_t::frame_size(utils::format_conversion::to_ffmpeg_format(pixel_format)
                                            , { size.width, size.height });
}

std::size_t video_format_t::planes(pixel_format_t pixel_format)
{
    return !is_encoded(pixel_format)
            ? ffmpeg::video_info_t::planes(utils::format_conversion::to_ffmpeg_format(pixel_format))
            : 1;
}

plane_sizes_t video_format_t::plane_sizes(pixel_format_t pixel_format
                                          , const frame_size_t& size)
{
    plane_sizes_t plane_sizes;

    for (const auto& sz : ffmpeg::video_info_t::plane_sizes(utils::format_conversion::to_ffmpeg_format(pixel_format)
                                                          , { size.width, size.height }))
    {
        plane_sizes.push_back(sz.size());
    }

    return plane_sizes;
}


std::size_t video_format_t::plane_width(pixel_format_t pixel_format
                                        , uint32_t width
                                        , uint32_t plane_idx)
{
    return ffmpeg::video_info_t::plane_width(utils::format_conversion::to_ffmpeg_format(pixel_format)
                                               , width
                                               , plane_idx);
}


video_format_t::video_format_t(pixel_format_t pixel_format
                               , frame_size_t size
                               , uint32_t fps)
    : media_format_t(media_type_t::video)
    , pixel_format(pixel_format)
    , size(size)
    , fps(fps)
{

}

bool video_format_t::operator ==(const video_format_t &video_format)
{
    return pixel_format == video_format.pixel_format
            && size == video_format.size
            && fps == video_format.fps;
}

bool video_format_t::operator !=(const video_format_t &video_format)
{
    return !operator ==(video_format);
}

bool video_format_t::is_planar() const
{
    return is_planar(pixel_format);
}

bool video_format_t::is_encoded() const
{
    return is_encoded(pixel_format);
}

std::size_t video_format_t::bpp() const
{
    return bpp(pixel_format);
}

std::size_t video_format_t::frame_size() const
{
    return frame_size(pixel_format
                      , size);
}

std::size_t video_format_t::planes() const
{
    return planes(pixel_format);
}

plane_sizes_t video_format_t::plane_sizes() const
{
    return plane_sizes(pixel_format
                       , size);
}

std::size_t video_format_t::plane_width(uint32_t plane_idx) const
{
    return plane_width(pixel_format
                       , size.width
                       , plane_idx);
}


}

}

}
