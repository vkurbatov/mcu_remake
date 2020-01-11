#include "format_converter.h"
#include <unordered_map>
#include <algorithm>

#include <linux/videodev2.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/pixfmt.h>
}


namespace core
{

namespace media
{

namespace utils
{

namespace format_conversion
{

typedef std::tuple<v4l2::pixel_format_t
                    , ffmpeg::codec_id_t
                    , ffmpeg::pixel_format_t
                    , std::string> format_desc_t;

const std::unordered_map<video::pixel_format_t
                , format_desc_t> format_table =
{
    { video::pixel_format_t::unknown,   { 0,                    AV_CODEC_ID_NONE,   AV_PIX_FMT_NONE,        "unk"       }   },
    { video::pixel_format_t::yuv420p,   { V4L2_PIX_FMT_YUV420,  AV_CODEC_ID_NONE,   AV_PIX_FMT_YUV420P,     "yuv420p"   }   },
    { video::pixel_format_t::yuv422p,   { V4L2_PIX_FMT_YUV422P, AV_CODEC_ID_NONE,   AV_PIX_FMT_YUV422P,     "yuv422p"   }   },
    { video::pixel_format_t::yuv444p,   { V4L2_PIX_FMT_YUV444M, AV_CODEC_ID_NONE,   AV_PIX_FMT_YUV444P,     "yuv444p"   }   },
    { video::pixel_format_t::yuyv,      { V4L2_PIX_FMT_YUYV,    AV_CODEC_ID_NONE,   AV_PIX_FMT_YUYV422,     "yuyv"      }   },
    { video::pixel_format_t::uyvy,      { V4L2_PIX_FMT_UYVY,    AV_CODEC_ID_NONE,   AV_PIX_FMT_UYVY422,     "uyvy"      }   },
    { video::pixel_format_t::yuv411p,   { V4L2_PIX_FMT_YUV411P, AV_CODEC_ID_NONE,   AV_PIX_FMT_YUV411P,     "yuv411p"   }   },
    { video::pixel_format_t::yuv410,    { V4L2_PIX_FMT_YVU410,  AV_CODEC_ID_NONE,   AV_PIX_FMT_YUV410P,     "yuv410"    }   },
    { video::pixel_format_t::rgb555,    { V4L2_PIX_FMT_RGB555,  AV_CODEC_ID_NONE,   AV_PIX_FMT_RGB555LE,    "rgb555"    }   },
    { video::pixel_format_t::rgb555x,   { V4L2_PIX_FMT_RGB555X, AV_CODEC_ID_NONE,   AV_PIX_FMT_RGB555BE,    "rgb555x"   }   },
    { video::pixel_format_t::rgb565,    { V4L2_PIX_FMT_RGB565,  AV_CODEC_ID_NONE,   AV_PIX_FMT_RGB565LE,    "rgb565"    }   },
    { video::pixel_format_t::rgb565x,   { V4L2_PIX_FMT_RGB565X, AV_CODEC_ID_NONE,   AV_PIX_FMT_RGB565BE,    "rgb565x"   }   },
    { video::pixel_format_t::bgr24,     { V4L2_PIX_FMT_BGR24,   AV_CODEC_ID_NONE,   AV_PIX_FMT_BGR24,       "bgr24"     }   },
    { video::pixel_format_t::rgb24,     { V4L2_PIX_FMT_RGB24,   AV_CODEC_ID_NONE,   AV_PIX_FMT_RGB24,       "rgb24"     }   },
    { video::pixel_format_t::bgr32,     { V4L2_PIX_FMT_ABGR32,  AV_CODEC_ID_NONE,   AV_PIX_FMT_BGRA,        "bgra"      }   },
    { video::pixel_format_t::rgb32,     { V4L2_PIX_FMT_ARGB32,  AV_CODEC_ID_NONE,   AV_PIX_FMT_ARGB,        "argb"      }   },
    { video::pixel_format_t::abgr32,    { V4L2_PIX_FMT_BGR32,   AV_CODEC_ID_NONE,   AV_PIX_FMT_BGR0,        "brg32"     }   },
    { video::pixel_format_t::argb32,    { V4L2_PIX_FMT_RGB32,   AV_CODEC_ID_NONE,   AV_PIX_FMT_0RGB,        "rgb32"     }   },
    { video::pixel_format_t::gray8,     { V4L2_PIX_FMT_GREY,    AV_CODEC_ID_NONE,   AV_PIX_FMT_GRAY8,       "gray8"     }   },
    { video::pixel_format_t::gray16,    { V4L2_PIX_FMT_Y16,     AV_CODEC_ID_NONE,   AV_PIX_FMT_GRAY16LE,    "gray16"    }   },
    { video::pixel_format_t::gray16x,   { V4L2_PIX_FMT_Y16_BE,  AV_CODEC_ID_NONE,   AV_PIX_FMT_GRAY16BE,    "gray16x"   }   },
    { video::pixel_format_t::nv12,      { V4L2_PIX_FMT_NV12,    AV_CODEC_ID_NONE,   AV_PIX_FMT_NV12,        "nv12"      }   },
    { video::pixel_format_t::sbggr8,    { V4L2_PIX_FMT_SBGGR8,  AV_CODEC_ID_NONE,   AV_PIX_FMT_BAYER_BGGR8, "bggr8"     }   },
    { video::pixel_format_t::sgbrg8,    { V4L2_PIX_FMT_SGBRG8,  AV_CODEC_ID_NONE,   AV_PIX_FMT_BAYER_GBRG8, "gbrg8"     }   },
    { video::pixel_format_t::sgrbg8,    { V4L2_PIX_FMT_SGRBG8,  AV_CODEC_ID_NONE,   AV_PIX_FMT_BAYER_GRBG8, "grbg8"     }   },
    { video::pixel_format_t::srggb8,    { V4L2_PIX_FMT_SRGGB8,  AV_CODEC_ID_NONE,   AV_PIX_FMT_BAYER_RGGB8, "rggb8"     }   },
    { video::pixel_format_t::mjpeg,     { V4L2_PIX_FMT_MJPEG,   AV_CODEC_ID_MJPEG,  AV_PIX_FMT_NONE,        "mjpeg"     }   },
    { video::pixel_format_t::h264,      { V4L2_PIX_FMT_H264,    AV_CODEC_ID_H264,   AV_PIX_FMT_NONE,        "h264"      }   },
    { video::pixel_format_t::h263,      { V4L2_PIX_FMT_H263,    AV_CODEC_ID_H263,   AV_PIX_FMT_NONE,        "h263"      }   },
    { video::pixel_format_t::vp8,       { V4L2_PIX_FMT_VP8,     AV_CODEC_ID_VP8,    AV_PIX_FMT_NONE,        "vp8"       }   },
    { video::pixel_format_t::vp9,       { V4L2_PIX_FMT_VP9,     AV_CODEC_ID_VP9,    AV_PIX_FMT_NONE,        "vp9"       }   },
    { video::pixel_format_t::mpeg4,     { V4L2_PIX_FMT_MPEG4,   AV_CODEC_ID_MPEG4,  AV_PIX_FMT_NONE,        "mpeg4"     }   },
    { video::pixel_format_t::cpia,      { V4L2_PIX_FMT_CPIA1,   AV_CODEC_ID_CPIA,   AV_PIX_FMT_NONE,        "cpia"      }   },
};


template<std::int32_t idx, typename T>
std::unordered_map<T, video::pixel_format_t> create_sub_table()
{
    std::unordered_map<T, video::pixel_format_t> sub_table;
    for(const auto& f : format_table)
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

const auto v4l2_format_table = create_sub_table<0, v4l2::pixel_format_t>();
const auto ffmpeg_codec_table = create_sub_table<1, ffmpeg::codec_id_t>();
const auto ffmpeg_format_table = create_sub_table<2, ffmpeg::pixel_format_t>();

const format_desc_t unknown_format = format_table.find(video::pixel_format_t::unknown)->second;

const format_desc_t& get_format_desc(video::pixel_format_t pixel_format)
{
    auto it = format_table.find(pixel_format);

    return it != format_table.end()
            ? it->second
            : unknown_format;
}

template <std::int32_t idx, typename T>
video::pixel_format_t get_pixel_format(const T& value)
{
    auto it = std::find_if(format_table.begin()
                        , format_table.end()
                        , [&value](const std::pair<video::pixel_format_t
                                                   , format_desc_t>& itm)
                        { return std::get<idx>(itm.second) == value; }
    );

    return it != format_table.end()
            ? it->first
            : video::pixel_format_t::unknown;

}

const std::string &get_format_name(video::pixel_format_t pixel_format)
{
    return std::get<3>(get_format_desc(pixel_format));
}

v4l2::pixel_format_t to_v4l2_format(video::pixel_format_t pixel_format)
{
    return std::get<0>(get_format_desc(pixel_format));
}

ffmpeg::pixel_format_t to_ffmpeg_format(video::pixel_format_t pixel_format)
{
    return std::get<2>(get_format_desc(pixel_format));
}

ffmpeg::codec_id_t to_ffmpeg_codec(video::pixel_format_t pixel_format)
{
    return std::get<1>(get_format_desc(pixel_format));
}

video::pixel_format_t form_v4l2_format(v4l2::pixel_format_t pixel_format)
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
        case V4L2_PIX_FMT_JPEG:
            pixel_format = V4L2_PIX_FMT_MJPEG;
        break;
    }

    auto it = v4l2_format_table.find(pixel_format);
    return it != v4l2_format_table.end()
            ? it->second
            :  video::pixel_format_t::unknown;

    // return get_pixel_format<0, v4l2::pixel_format_t>(pixel_format);
}

video::pixel_format_t from_ffmpeg_format(ffmpeg::pixel_format_t pixel_format)
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

    auto it = ffmpeg_format_table.find(pixel_format);
    return it != ffmpeg_format_table.end()
            ? it->second
            :  video::pixel_format_t::unknown;

    // return get_pixel_format<1>(pixel_format);
}

video::pixel_format_t from_ffmpeg_codec(ffmpeg::codec_id_t codec_id)
{
    switch (codec_id)
    {
        case AV_CODEC_ID_RAWVIDEO:
            codec_id = AV_CODEC_ID_NONE;
        break;
    }

    auto it = ffmpeg_codec_table.find(codec_id);
    return it != ffmpeg_codec_table.end()
            ? it->second
            :  video::pixel_format_t::unknown;

    // return get_pixel_format<2>(codec_id);
}

}

}

}

}
