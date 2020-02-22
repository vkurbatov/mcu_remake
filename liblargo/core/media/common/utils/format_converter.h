#ifndef FORMAT_CONVERTER_H
#define FORMAT_CONVERTER_H

#include "media/common/ffmpeg/libav_base.h"
#include "media/common/v4l2/v4l2_base.h"
#include "media/video/video_format.h"
#include "media/audio/audio_format.h"

namespace core
{

namespace media
{

namespace alsa
{
typedef std::int32_t sample_format_t;
}

namespace utils
{


namespace format_conversion
{   


    const std::string &get_format_name(video::pixel_format_t pixel_format);
    const std::string &get_format_name(audio::sample_format_t sample_format);

    v4l2::pixel_format_t to_v4l2_video_format(video::pixel_format_t pixel_format);
    ffmpeg::pixel_format_t to_ffmpeg_video_format(video::pixel_format_t pixel_format);
    ffmpeg::codec_id_t to_ffmpeg_video_codec(video::pixel_format_t pixel_format);

    alsa::sample_format_t to_alsa_audio_format(audio::sample_format_t sample_format);
    ffmpeg::sample_format_t to_ffmpeg_audio_format(audio::sample_format_t sample_format);
    ffmpeg::codec_id_t to_ffmpeg_audio_codec(audio::sample_format_t sample_format);

    video::pixel_format_t  from_v4l2_video_format(v4l2::pixel_format_t pixel_format);
    video::pixel_format_t  from_ffmpeg_video_format(ffmpeg::pixel_format_t pixel_format);
    video::pixel_format_t  from_ffmpeg_video_codec(ffmpeg::codec_id_t codec_id);

    audio::sample_format_t  from_alsa_audio_format(alsa::sample_format_t sample_format);
    audio::sample_format_t  from_ffmpeg_audio_format(ffmpeg::sample_format_t sample_format);
    audio::sample_format_t  from_ffmpeg_audio_codec(ffmpeg::codec_id_t codec_id);
}

}

}

}

#endif // FORMAT_CONVERTER_H
