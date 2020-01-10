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

video_format_t::video_format_t(pixel_format_t pixel_format
                               , frame_size_t size
                               , uint32_t fps)
    : pixel_format(pixel_format)
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

}

}

}
