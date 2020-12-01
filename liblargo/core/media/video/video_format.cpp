#include "video_format.h"
#include "media/common/utils/format_converter.h"
#include "media/common/ffmpeg/libav_base.h"

#include <sstream>


namespace core
{

namespace media
{

namespace video
{

//-----------------------------------------------------------------------------------------------------------

bool video_info_t::is_planar(pixel_format_t pixel_format)
{
    return pixel_format < pixel_format_t::bgr555;
}

bool video_info_t::is_encoded(pixel_format_t pixel_format)
{
    return pixel_format >= pixel_format_t::jpeg;
}

std::size_t video_info_t::bpp(pixel_format_t pixel_format)
{
    return ffmpeg::video_info_t::bpp(utils::format_conversion::to_ffmpeg_video_format(pixel_format));
}

std::size_t video_info_t::frame_size(pixel_format_t pixel_format
                                       , const frame_size_t &size)
{
    return ffmpeg::video_info_t::frame_size(utils::format_conversion::to_ffmpeg_video_format(pixel_format)
                                            , { size.width, size.height });
}

std::size_t video_info_t::planes(pixel_format_t pixel_format)
{
    return !is_encoded(pixel_format)
            ? ffmpeg::video_info_t::planes(utils::format_conversion::to_ffmpeg_video_format(pixel_format))
            : 1;
}

plane_sizes_t video_info_t::plane_sizes(pixel_format_t pixel_format
                                          , const frame_size_t& size)
{
    plane_sizes_t plane_sizes;

    for (const auto& sz : ffmpeg::video_info_t::plane_sizes(utils::format_conversion::to_ffmpeg_video_format(pixel_format)
                                                          , { size.width, size.height }))
    {
        plane_sizes.push_back(sz.size());
    }

    return plane_sizes;
}

frame_size_t video_info_t::plane_size(pixel_format_t pixel_format
                                        , const frame_size_t& size
                                        , uint32_t plane_idx)
{
    auto sizes = ffmpeg::video_info_t::plane_sizes(utils::format_conversion::to_ffmpeg_video_format(pixel_format)
                                                              , { size.width, size.height });

    return plane_idx < sizes.size()
            ? frame_size_t(sizes[plane_idx].width, sizes[plane_idx].height)
            : frame_size_t();
}

std::string video_info_t::to_string(pixel_format_t pixel_format
                                      , frame_size_t size
                                      , uint32_t fps)
{
    return static_cast<std::stringstream&>(std::stringstream()
                                           << size.width
                                           << "x"
                                           << size.height
                                           << "@"
                                           << fps
                                           << ":"
                                           << media::utils::format_conversion::get_format_name(pixel_format)).str();
}

pixel_format_t video_info_t::raw_pixel_format(pixel_format_t pixel_format)
{
    if (is_encoded(pixel_format))
    {
        return utils::format_conversion::from_ffmpeg_video_format(utils::format_conversion::to_ffmpeg_video_format(pixel_format));
    }
    return pixel_format;
}

video_info_t::video_info_t(pixel_format_t pixel_format
                               , frame_size_t size
                               , uint32_t fps)
    : pixel_format(pixel_format)
    , size(size)
    , fps(fps)
{

}

bool video_info_t::operator ==(const video_info_t& video_info) const
{
    return pixel_format == video_info.pixel_format
                    && size == video_info.size
                    && fps == video_info.fps;
}

bool video_info_t::operator !=(const video_info_t& video_info) const
{
    return !operator ==(video_info);
}

bool video_info_t::is_planar() const
{
    return is_planar(pixel_format);
}

bool video_info_t::is_encoded() const
{
    return is_encoded(pixel_format);
}

bool video_info_t::is_convertable() const
{
    return !is_encoded()
            && pixel_format != pixel_format_t::unknown;
}

std::size_t video_info_t::bpp() const
{
    return bpp(pixel_format);
}

std::size_t video_info_t::frame_size() const
{
    return frame_size(pixel_format
                      , size);
}

std::size_t video_info_t::planes() const
{
    return planes(pixel_format);
}

plane_sizes_t video_info_t::plane_sizes() const
{
    return plane_sizes(pixel_format
                       , size);
}

pixel_format_t video_info_t::raw_pixel_format() const
{
    return raw_pixel_format(pixel_format);
}


frame_size_t video_info_t::plane_size(uint32_t plane_idx) const
{
    return plane_size(pixel_format
                      , size
                      , plane_idx);
}

std::string video_info_t::to_string() const
{
    return to_string(pixel_format
                     , size
                     , fps);
}

}

}

}
