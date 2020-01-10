#ifndef FORMAT_CONVERTER_H
#define FORMAT_CONVERTER_H

#include "media/common/ffmpeg/libav_base.h"
#include "media/common/v4l2/v4l2_base.h"
#include "media/video/video_format.h"

namespace core
{

namespace media
{

namespace utils
{

namespace format_conversion
{

    const std::string &get_format_name(video::pixel_format_t pixel_format);

    v4l2::pixel_format_t to_v4l2_format(video::pixel_format_t pixel_format);
    ffmpeg::pixel_format_t to_ffmpeg_codec(video::pixel_format_t pixel_format);
    ffmpeg::codec_id_t to_ffmpeg_format(video::pixel_format_t pixel_format);

    video::pixel_format_t  form_v4l2_format(v4l2::pixel_format_t pixel_format);
    video::pixel_format_t  from_ffmpeg_codec(ffmpeg::pixel_format_t pixel_format);
    video::pixel_format_t  from_ffmpeg_format(ffmpeg::codec_id_t codec_id);
}

}

}

}

#endif // FORMAT_CONVERTER_H
