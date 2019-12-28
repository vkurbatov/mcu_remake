#include "format_converter.h"
#include <unordered_map>

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

struct ffmpeg_format_desc_t
{
    ffmpeg::codec_id_t codec_id;
    ffmpeg::pixel_format_t pixel_format;
    std::string name;
};

const std::unordered_map<v4l2::pixel_format_t
                , ffmpeg_format_desc_t> format_table =
{
    { V4L2_PIX_FMT_YUV420,  { AV_CODEC_ID_RAWVIDEO, AV_PIX_FMT_YUV420P,     "yuv420p"   }   },
    { V4L2_PIX_FMT_YUV422P, { AV_CODEC_ID_RAWVIDEO, AV_PIX_FMT_YUV422P,     "yuv422p"   }   },
    { V4L2_PIX_FMT_YUYV,    { AV_CODEC_ID_RAWVIDEO, AV_PIX_FMT_YUYV422,     "yuyv"      }   },
    { V4L2_PIX_FMT_UYVY,    { AV_CODEC_ID_RAWVIDEO, AV_PIX_FMT_UYVY422,     "uyvy"      }   },
    { V4L2_PIX_FMT_YUV411P, { AV_CODEC_ID_RAWVIDEO, AV_PIX_FMT_YUV411P,     "yuv411p"   }   },
    { V4L2_PIX_FMT_YVU410,  { AV_CODEC_ID_RAWVIDEO, AV_PIX_FMT_YUV410P,     "yuv410"    }   },
    { V4L2_PIX_FMT_RGB555,  { AV_CODEC_ID_RAWVIDEO, AV_PIX_FMT_RGB555LE,    "rgb555"    }   },
    { V4L2_PIX_FMT_RGB555X, { AV_CODEC_ID_RAWVIDEO, AV_PIX_FMT_RGB555BE,    "rgb555x"   }   },
    { V4L2_PIX_FMT_RGB565,  { AV_CODEC_ID_RAWVIDEO, AV_PIX_FMT_RGB565LE,    "rgb565"    }   },
    { V4L2_PIX_FMT_RGB565X, { AV_CODEC_ID_RAWVIDEO, AV_PIX_FMT_RGB565BE,    "rgb565x"   }   },
    { V4L2_PIX_FMT_BGR24,   { AV_CODEC_ID_RAWVIDEO, AV_PIX_FMT_BGR24,       "bgr24"     }   },
    { V4L2_PIX_FMT_RGB24,   { AV_CODEC_ID_RAWVIDEO, AV_PIX_FMT_RGB24,       "rgb24"     }   },
#ifdef V4L2_PIX_FMT_XBGR32
    { V4L2_PIX_FMT_XBGR32,  { AV_CODEC_ID_RAWVIDEO, AV_PIX_FMT_BGR0,        "bgrx"      }   },
    { V4L2_PIX_FMT_XRGB32,  { AV_CODEC_ID_RAWVIDEO, AV_PIX_FMT_0RGB,        "xrgb"      }   },
    { V4L2_PIX_FMT_ABGR32,  { AV_CODEC_ID_RAWVIDEO, AV_PIX_FMT_BGRA,        "bgra"      }   },
    { V4L2_PIX_FMT_ARGB32,  { AV_CODEC_ID_RAWVIDEO, AV_PIX_FMT_ARGB,        "argb"      }   },
#endif
    { V4L2_PIX_FMT_BGR32,   { AV_CODEC_ID_RAWVIDEO, AV_PIX_FMT_BGR0,        "brg32"     }   },
    { V4L2_PIX_FMT_RGB32,   { AV_CODEC_ID_RAWVIDEO, AV_PIX_FMT_0RGB,        "rgb32"     }   },
    { V4L2_PIX_FMT_GREY,    { AV_CODEC_ID_RAWVIDEO, AV_PIX_FMT_GRAY8,       "gray"      }   },
#ifdef V4L2_PIX_FMT_Y16
    { V4L2_PIX_FMT_Y16,     { AV_CODEC_ID_RAWVIDEO, AV_PIX_FMT_GRAY16LE,    "gy16"      }   },
#endif
#ifdef V4L2_PIX_FMT_Z16
    { V4L2_PIX_FMT_Z16,     { AV_CODEC_ID_RAWVIDEO, AV_PIX_FMT_GRAY16LE,    "gz16"      }   },
#endif
    { V4L2_PIX_FMT_NV12,    { AV_CODEC_ID_RAWVIDEO, AV_PIX_FMT_NV12,        "nv12"      }   },
    { V4L2_PIX_FMT_SBGGR8,  { AV_CODEC_ID_RAWVIDEO, AV_PIX_FMT_BAYER_BGGR8, "bggr8"     }   },
    { V4L2_PIX_FMT_SGBRG8,  { AV_CODEC_ID_RAWVIDEO, AV_PIX_FMT_BAYER_GBRG8, "gbrg8"     }   },
    { V4L2_PIX_FMT_SGRBG8,  { AV_CODEC_ID_RAWVIDEO, AV_PIX_FMT_BAYER_GRBG8, "grbg8"     }   },
    { V4L2_PIX_FMT_SRGGB8,  { AV_CODEC_ID_RAWVIDEO, AV_PIX_FMT_BAYER_RGGB8, "rggb8"     }   },
    { V4L2_PIX_FMT_MJPEG,   { AV_CODEC_ID_MJPEG,    AV_PIX_FMT_NONE,        "mjpeg"     }   },
    { V4L2_PIX_FMT_JPEG,    { AV_CODEC_ID_MJPEG,    AV_PIX_FMT_NONE,        "jpeg"      }   },
    { V4L2_PIX_FMT_H264,    { AV_CODEC_ID_H264,     AV_PIX_FMT_NONE,        "h264"      }   },
    { V4L2_PIX_FMT_H263,    { AV_CODEC_ID_H263,     AV_PIX_FMT_NONE,        "h263"      }   },
    { V4L2_PIX_FMT_VP8,     { AV_CODEC_ID_VP8,      AV_PIX_FMT_NONE,        "vp8"       }   },
    { V4L2_PIX_FMT_VP9,     { AV_CODEC_ID_VP9,      AV_PIX_FMT_NONE,        "vp9"       }   },
#ifdef V4L2_PIX_FMT_MPEG4
    { V4L2_PIX_FMT_MPEG4,   { AV_CODEC_ID_MPEG4,    AV_PIX_FMT_NONE,        "mpeg4"     }   },
#endif
#ifdef V4L2_PIX_FMT_CPIA1
    { V4L2_PIX_FMT_CPIA1,   { AV_CODEC_ID_CPIA,     AV_PIX_FMT_NONE,        "cpia"      }   },
#endif
};

static const ffmpeg_format_desc_t& get_format_desc(v4l2::pixel_format_t pixel_format)
{
    static ffmpeg_format_desc_t unknown_desc = { ffmpeg::unknown_codec_id
                                , ffmpeg::unknown_pixel_format
                                , "unk" };

    auto it = format_table.find(pixel_format);

    return it != format_table.end()
            ? it->second
            : unknown_desc;
}

ffmpeg::codec_id_t ffmpeg_codec_id_from_v4l2_format(v4l2::pixel_format_t pixel_format)
{
    return get_format_desc(pixel_format).codec_id;
}

ffmpeg::pixel_format_t ffmpeg_format_from_v4l2_format(v4l2::pixel_format_t pixel_format)
{
    return get_format_desc(pixel_format).pixel_format;
}

const std::string &format_name_from_v4l2_format(v4l2::pixel_format_t pixel_format)
{
    return get_format_desc(pixel_format).name;
}

}

}

}
