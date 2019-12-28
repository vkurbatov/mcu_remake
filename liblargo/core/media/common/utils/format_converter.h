#ifndef FORMAT_CONVERTER_H
#define FORMAT_CONVERTER_H

#include "media/common/ffmpeg/libav_base.h"
#include "media/common/v4l2/v4l2_base.h"

namespace core
{

namespace media
{

namespace utils
{

    ffmpeg::codec_id_t ffmpeg_codec_id_from_v4l2_format(v4l2::pixel_format_t pixel_format);
    ffmpeg::pixel_format_t ffmpeg_format_from_v4l2_format(v4l2::pixel_format_t pixel_format);
    const std::string &format_name_from_v4l2_format(v4l2::pixel_format_t pixel_format);

}

}

}

#endif // FORMAT_CONVERTER_H
