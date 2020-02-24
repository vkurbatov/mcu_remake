#include "format_converter.h"
#include <unordered_map>
#include <algorithm>

#include <linux/videodev2.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/pixfmt.h>
}

extern "C"
{
#include <alsa/asoundlib.h>
}


namespace core
{

namespace media
{

namespace utils
{

namespace format_conversion
{

typedef std::tuple<alsa::sample_format_t
                   , ffmpeg::codec_id_t
                   , ffmpeg::sample_format_t
                   , std::string> audio_format_desc_t;

const std::unordered_map<audio::sample_format_t
                , audio_format_desc_t> audio_format_table =
{
    { audio::sample_format_t::unknown,  { SND_PCM_FORMAT_UNKNOWN,   AV_CODEC_ID_NONE,       AV_SAMPLE_FMT_NONE,     "unk"     }   },
    { audio::sample_format_t::pcm_8,    { SND_PCM_FORMAT_S8,        AV_CODEC_ID_NONE,       AV_SAMPLE_FMT_U8,       "pcm8"    }   },
    { audio::sample_format_t::pcm_16,   { SND_PCM_FORMAT_S16,       AV_CODEC_ID_NONE,       AV_SAMPLE_FMT_S16,      "pcm16"   }   },
    { audio::sample_format_t::pcm_32,   { SND_PCM_FORMAT_S32,       AV_CODEC_ID_NONE,       AV_SAMPLE_FMT_S32,      "pcm32"   }   },
    { audio::sample_format_t::float_32, { SND_PCM_FORMAT_FLOAT,     AV_CODEC_ID_NONE,       AV_SAMPLE_FMT_FLT,      "flt32"   }   },
    { audio::sample_format_t::float_64, { SND_PCM_FORMAT_FLOAT64,   AV_CODEC_ID_NONE,       AV_SAMPLE_FMT_DBL,      "flt64"   }   },
    { audio::sample_format_t::aac,      { SND_PCM_FORMAT_UNKNOWN,   AV_CODEC_ID_AAC,        AV_SAMPLE_FMT_NONE,     "aac"     }   },
    { audio::sample_format_t::pcma,     { SND_PCM_FORMAT_A_LAW,     AV_CODEC_ID_PCM_ALAW,   AV_SAMPLE_FMT_NONE,     "pcma"    }   },
    { audio::sample_format_t::pcmu,     { SND_PCM_FORMAT_MU_LAW,    AV_CODEC_ID_PCM_MULAW,  AV_SAMPLE_FMT_NONE,     "pcmu"    }   }
};

typedef std::tuple<v4l2::pixel_format_t
                    , ffmpeg::codec_id_t
                    , ffmpeg::pixel_format_t
                    , std::string> video_format_desc_t;

const std::unordered_map<video::pixel_format_t
                , video_format_desc_t> video_format_table =
{
    { video::pixel_format_t::unknown,   { 0,                    AV_CODEC_ID_NONE,       AV_PIX_FMT_NONE,        "unk"       }   },
    { video::pixel_format_t::yuv420p,   { V4L2_PIX_FMT_YUV420,  AV_CODEC_ID_NONE,       AV_PIX_FMT_YUV420P,     "yuv420p"   }   },
    { video::pixel_format_t::yuv422p,   { V4L2_PIX_FMT_YUV422P, AV_CODEC_ID_NONE,       AV_PIX_FMT_YUV422P,     "yuv422p"   }   },
    { video::pixel_format_t::yuv444p,   { V4L2_PIX_FMT_YUV444M, AV_CODEC_ID_NONE,       AV_PIX_FMT_YUV444P,     "yuv444p"   }   },
    { video::pixel_format_t::yuyv,      { V4L2_PIX_FMT_YUYV,    AV_CODEC_ID_NONE,       AV_PIX_FMT_YUYV422,     "yuyv"      }   },
    { video::pixel_format_t::uyvy,      { V4L2_PIX_FMT_UYVY,    AV_CODEC_ID_NONE,       AV_PIX_FMT_UYVY422,     "uyvy"      }   },
    { video::pixel_format_t::yuv411p,   { V4L2_PIX_FMT_YUV411P, AV_CODEC_ID_NONE,       AV_PIX_FMT_YUV411P,     "yuv411p"   }   },
    { video::pixel_format_t::yuv410,    { V4L2_PIX_FMT_YVU410,  AV_CODEC_ID_NONE,       AV_PIX_FMT_YUV410P,     "yuv410"    }   },
    { video::pixel_format_t::bgr555,    { 0,                    AV_CODEC_ID_NONE,       AV_PIX_FMT_BGR555LE,    "bgr555"    }   },
    { video::pixel_format_t::bgr555x,   { 0,                    AV_CODEC_ID_NONE,       AV_PIX_FMT_BGR555BE,    "bgr555x"   }   },
    { video::pixel_format_t::bgr565,    { 0,                    AV_CODEC_ID_NONE,       AV_PIX_FMT_BGR565LE,    "bgr565"    }   },
    { video::pixel_format_t::bgr565x,   { 0,                    AV_CODEC_ID_NONE,       AV_PIX_FMT_BGR565BE,    "bgr565x"   }   },
    { video::pixel_format_t::rgb555,    { V4L2_PIX_FMT_RGB555,  AV_CODEC_ID_NONE,       AV_PIX_FMT_RGB555LE,    "rgb555"    }   },
    { video::pixel_format_t::rgb555x,   { V4L2_PIX_FMT_RGB555X, AV_CODEC_ID_NONE,       AV_PIX_FMT_RGB555BE,    "rgb555x"   }   },
    { video::pixel_format_t::rgb565,    { V4L2_PIX_FMT_RGB565,  AV_CODEC_ID_NONE,       AV_PIX_FMT_RGB565LE,    "rgb565"    }   },
    { video::pixel_format_t::rgb565x,   { V4L2_PIX_FMT_RGB565X, AV_CODEC_ID_NONE,       AV_PIX_FMT_RGB565BE,    "rgb565x"   }   },
    { video::pixel_format_t::bgr8,      { 0,                    AV_CODEC_ID_NONE,       AV_PIX_FMT_BGR8,        "rgb8"      }   },
    { video::pixel_format_t::rgb8,      { V4L2_PIX_FMT_RGB332,  AV_CODEC_ID_NONE,       AV_PIX_FMT_RGB8,        "bgr8"      }   },
    { video::pixel_format_t::bgr24,     { V4L2_PIX_FMT_BGR24,   AV_CODEC_ID_NONE,       AV_PIX_FMT_BGR24,       "bgr24"     }   },
    { video::pixel_format_t::rgb24,     { V4L2_PIX_FMT_RGB24,   AV_CODEC_ID_NONE,       AV_PIX_FMT_RGB24,       "rgb24"     }   },
    { video::pixel_format_t::abgr32,    { V4L2_PIX_FMT_ABGR32,  AV_CODEC_ID_NONE,       AV_PIX_FMT_ABGR,        "abrg"      }   },
    { video::pixel_format_t::argb32,    { V4L2_PIX_FMT_ARGB32,  AV_CODEC_ID_NONE,       AV_PIX_FMT_ARGB,        "argb"      }   },
    { video::pixel_format_t::bgra32,    { 0,                    AV_CODEC_ID_NONE,       AV_PIX_FMT_BGRA,        "brga"      }   },
    { video::pixel_format_t::rgba32,    { 0,                    AV_CODEC_ID_NONE,       AV_PIX_FMT_RGBA,        "rgba"      }   },
    { video::pixel_format_t::bgr32,     { V4L2_PIX_FMT_BGR32,   AV_CODEC_ID_NONE,       AV_PIX_FMT_BGR0,        "brg32"     }   },
    { video::pixel_format_t::rgb32,     { V4L2_PIX_FMT_RGB32,   AV_CODEC_ID_NONE,       AV_PIX_FMT_RGB0,        "rgb32"     }   },
    { video::pixel_format_t::gray8,     { V4L2_PIX_FMT_GREY,    AV_CODEC_ID_NONE,       AV_PIX_FMT_GRAY8,       "gray8"     }   },
    { video::pixel_format_t::gray16,    { V4L2_PIX_FMT_Y16,     AV_CODEC_ID_NONE,       AV_PIX_FMT_GRAY16LE,    "gray16"    }   },
    { video::pixel_format_t::gray16x,   { V4L2_PIX_FMT_Y16_BE,  AV_CODEC_ID_NONE,       AV_PIX_FMT_GRAY16BE,    "gray16x"   }   },
    { video::pixel_format_t::nv12,      { V4L2_PIX_FMT_NV12,    AV_CODEC_ID_NONE,       AV_PIX_FMT_NV12,        "nv12"      }   },
    { video::pixel_format_t::nv21,      { V4L2_PIX_FMT_NV21,    AV_CODEC_ID_NONE,       AV_PIX_FMT_NV21,        "nv21"      }   },
    { video::pixel_format_t::nv16,      { V4L2_PIX_FMT_NV16,    AV_CODEC_ID_NONE,       AV_PIX_FMT_NV16,        "nv16"      }   },
    { video::pixel_format_t::sbggr8,    { V4L2_PIX_FMT_SBGGR8,  AV_CODEC_ID_NONE,       AV_PIX_FMT_BAYER_BGGR8, "bggr8"     }   },
    { video::pixel_format_t::sgbrg8,    { V4L2_PIX_FMT_SGBRG8,  AV_CODEC_ID_NONE,       AV_PIX_FMT_BAYER_GBRG8, "gbrg8"     }   },
    { video::pixel_format_t::sgrbg8,    { V4L2_PIX_FMT_SGRBG8,  AV_CODEC_ID_NONE,       AV_PIX_FMT_BAYER_GRBG8, "grbg8"     }   },
    { video::pixel_format_t::srggb8,    { V4L2_PIX_FMT_SRGGB8,  AV_CODEC_ID_NONE,       AV_PIX_FMT_BAYER_RGGB8, "rggb8"     }   },
    { video::pixel_format_t::jpeg,      { V4L2_PIX_FMT_JPEG,    AV_CODEC_ID_JPEG2000,   AV_PIX_FMT_NONE,        "jpeg"     }   },
    { video::pixel_format_t::mjpeg,     { V4L2_PIX_FMT_MJPEG,   AV_CODEC_ID_MJPEG,      AV_PIX_FMT_NONE,        "mjpeg"     }   },
    { video::pixel_format_t::h264,      { V4L2_PIX_FMT_H264,    AV_CODEC_ID_H264,       AV_PIX_FMT_NONE,        "h264"      }   },
    { video::pixel_format_t::h263,      { V4L2_PIX_FMT_H263,    AV_CODEC_ID_H263,       AV_PIX_FMT_NONE,        "h263"      }   },
    { video::pixel_format_t::vp8,       { V4L2_PIX_FMT_VP8,     AV_CODEC_ID_VP8,        AV_PIX_FMT_NONE,        "vp8"       }   },
    { video::pixel_format_t::vp9,       { V4L2_PIX_FMT_VP9,     AV_CODEC_ID_VP9,        AV_PIX_FMT_NONE,        "vp9"       }   },
    { video::pixel_format_t::mpeg4,     { V4L2_PIX_FMT_MPEG4,   AV_CODEC_ID_MPEG4,      AV_PIX_FMT_NONE,        "mpeg4"     }   },
    { video::pixel_format_t::cpia,      { V4L2_PIX_FMT_CPIA1,   AV_CODEC_ID_CPIA,       AV_PIX_FMT_NONE,        "cpia"      }   },
};

template<std::int32_t idx, typename T>
std::unordered_map<T, video::pixel_format_t> create_video_sub_table()
{
    std::unordered_map<T, video::pixel_format_t> sub_table;
    for(const auto& f : video_format_table)
    {
        const auto& val = std::get<idx>(f.second);
        if (sub_table.find(val) == sub_table.end())
        {
            sub_table.emplace(val
                              , f.first);
        }
    }

    return std::move(sub_table);
}

template<std::int32_t idx, typename T>
std::unordered_map<T, audio::sample_format_t> create_audio_sub_table()
{
    std::unordered_map<T, audio::sample_format_t> sub_table;
    for(const auto& f : audio_format_table)
    {
        const auto& val = std::get<idx>(f.second);
        if (sub_table.find(val) == sub_table.end())
        {
            sub_table.emplace(val
                              , f.first);
        }
    }

    return std::move(sub_table);
}

const auto v4l2_video_format_table = create_video_sub_table<0, v4l2::pixel_format_t>();
const auto ffmpeg_video_codec_table = create_video_sub_table<1, ffmpeg::codec_id_t>();
const auto ffmpeg_video_format_table = create_video_sub_table<2, ffmpeg::pixel_format_t>();

const auto alsa_audio_format_table = create_audio_sub_table<0, alsa::sample_format_t>();
const auto ffmpeg_audio_codec_table = create_audio_sub_table<1, ffmpeg::codec_id_t>();
const auto ffmpeg_audio_format_table = create_audio_sub_table<2, ffmpeg::sample_format_t>();


const video_format_desc_t unknown_video_format = video_format_table.find(video::pixel_format_t::unknown)->second;
const audio_format_desc_t unknown_audio_format = audio_format_table.find(audio::sample_format_t::unknown)->second;

const video_format_desc_t& get_format_desc(video::pixel_format_t pixel_format)
{
    auto it = video_format_table.find(pixel_format);

    return it != video_format_table.end()
            ? it->second
            : unknown_video_format;
}

const audio_format_desc_t& get_format_desc(audio::sample_format_t sample_format)
{
    auto it = audio_format_table.find(sample_format);

    return it != audio_format_table.end()
            ? it->second
            : unknown_audio_format;
}

template <std::int32_t idx, typename T>
video::pixel_format_t get_pixel_format(const T& value)
{
    auto it = std::find_if(video_format_table.begin()
                        , video_format_table.end()
                        , [&value](const std::pair<video::pixel_format_t
                                                   , video_format_desc_t>& itm)
                        { return std::get<idx>(itm.second) == value; }
    );

    return it != video_format_table.end()
            ? it->first
            : video::pixel_format_t::unknown;

}

const std::string &get_format_name(video::pixel_format_t pixel_format)
{
    return std::get<3>(get_format_desc(pixel_format));
}

v4l2::pixel_format_t to_v4l2_video_format(video::pixel_format_t pixel_format)
{
    return std::get<0>(get_format_desc(pixel_format));
}

ffmpeg::pixel_format_t to_ffmpeg_video_format(video::pixel_format_t pixel_format)
{        
    return std::get<2>(get_format_desc(pixel_format));
}

ffmpeg::codec_id_t to_ffmpeg_video_codec(video::pixel_format_t pixel_format)
{
    return std::get<1>(get_format_desc(pixel_format));
}

alsa::sample_format_t to_alsa_audio_format(audio::sample_format_t sample_format)
{
    return std::get<0>(get_format_desc(sample_format));
}

ffmpeg::sample_format_t to_ffmpeg_audio_format(audio::sample_format_t sample_format)
{
    return std::get<2>(get_format_desc(sample_format));
}

ffmpeg::codec_id_t to_ffmpeg_audio_codec(audio::sample_format_t sample_format)
{
    return std::get<1>(get_format_desc(sample_format));
}

video::pixel_format_t from_v4l2_video_format(v4l2::pixel_format_t pixel_format)
{
    switch (pixel_format)
    {
        case V4L2_PIX_FMT_XBGR32:
            pixel_format = V4L2_PIX_FMT_BGR32;
        break;
        case V4L2_PIX_FMT_XRGB32:
            pixel_format = V4L2_PIX_FMT_RGB32;
        break;
        case V4L2_PIX_FMT_Z16:
            pixel_format = V4L2_PIX_FMT_Y16;
        break;
    }

    auto it = v4l2_video_format_table.find(pixel_format);
    return it != v4l2_video_format_table.end()
            ? it->second
            :  video::pixel_format_t::unknown;

    // return get_pixel_format<0, v4l2::pixel_format_t>(pixel_format);
}

video::pixel_format_t from_ffmpeg_video_format(ffmpeg::pixel_format_t pixel_format)
{
    switch (pixel_format)
    {
        case AV_PIX_FMT_YUVJ420P:
            pixel_format = AV_PIX_FMT_YUV420P;
        break;
        case AV_PIX_FMT_YUVJ422P:
            pixel_format = AV_PIX_FMT_YUV422P;
        break;
        case AV_PIX_FMT_YUVJ444P:
            pixel_format = AV_PIX_FMT_YUV444P;
        break;
    }

    auto it = ffmpeg_video_format_table.find(pixel_format);
    return it != ffmpeg_video_format_table.end()
            ? it->second
            :  video::pixel_format_t::unknown;

    // return get_pixel_format<1>(pixel_format);
}

video::pixel_format_t from_ffmpeg_video_codec(ffmpeg::codec_id_t codec_id)
{
    switch (codec_id)
    {
        case AV_CODEC_ID_RAWVIDEO:
            codec_id = AV_CODEC_ID_NONE;
        break;
    }

    auto it = ffmpeg_video_codec_table.find(codec_id);
    return it != ffmpeg_video_codec_table.end()
            ? it->second
            :  video::pixel_format_t::unknown;

    // return get_pixel_format<2>(codec_id);
}

const std::string &get_format_name(audio::sample_format_t sample_format)
{
    return std::get<3>(get_format_desc(sample_format));
}


audio::sample_format_t from_alsa_audio_format(alsa::sample_format_t sample_format)
{
    auto it = alsa_audio_format_table.find(sample_format);
    return it != alsa_audio_format_table.end()
            ? it->second
            :  audio::sample_format_t::unknown;
}

audio::sample_format_t from_ffmpeg_audio_format(ffmpeg::sample_format_t sample_format)
{
    auto it = ffmpeg_audio_format_table.find(sample_format);
    return it != ffmpeg_audio_format_table.end()
            ? it->second
            :  audio::sample_format_t::unknown;
}

audio::sample_format_t from_ffmpeg_audio_codec(ffmpeg::codec_id_t codec_id)
{
    auto it = ffmpeg_audio_codec_table.find(codec_id);
    return it != ffmpeg_audio_codec_table.end()
            ? it->second
            :  audio::sample_format_t::unknown;
}

}

}

}

}
