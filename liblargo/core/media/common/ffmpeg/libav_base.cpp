#include "libav_base.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
}

#include <sstream>

namespace ffmpeg
{

const codec_id_t codec_id_h264 = static_cast<codec_id_t>(AV_CODEC_ID_H264);
const codec_id_t codec_id_mjpeg = static_cast<codec_id_t>(AV_CODEC_ID_MJPEG);
const codec_id_t codec_id_raw_video = static_cast<codec_id_t>(AV_CODEC_ID_RAWVIDEO);
const codec_id_t codec_id_none = static_cast<codec_id_t>(AV_CODEC_ID_NONE);

//extern const pixel_format_t default_pixel_format = static_cast<pixel_format_t>(AV_PIX_FMT_YUV420P);
const pixel_format_t default_pixel_format = static_cast<pixel_format_t>(AV_PIX_FMT_YUV420P);
const sample_format_t default_sample_format = static_cast<sample_format_t>(AV_SAMPLE_FMT_S16);

const pixel_format_t pixel_format_bgr24 = static_cast<pixel_format_t>(AV_PIX_FMT_BGR24);
const pixel_format_t pixel_format_rgb24 = static_cast<pixel_format_t>(AV_PIX_FMT_RGB24);
const pixel_format_t pixel_format_yuv420p = static_cast<pixel_format_t>(AV_PIX_FMT_YUV420P);
const pixel_format_t pixel_format_none = static_cast<pixel_format_t>(AV_PIX_FMT_NONE);


std::string error_to_string(int32_t av_error)
{
    char err[AV_ERROR_MAX_STRING_SIZE] = {};
    av_strerror(av_error, err, AV_ERROR_MAX_STRING_SIZE);
    return err;
}



//const codec_id_t codec_id_yuv420p = static_cast<codec_id_t>(AV_CODEC_ID_NONE);


option_format_t  device_option_t::option_format(option_type_t type)
{
    switch(static_cast<AVOptionType>(type))
    {
        case AV_OPT_TYPE_FLAGS:
        case AV_OPT_TYPE_INT:
        case AV_OPT_TYPE_INT64:
        case AV_OPT_TYPE_BOOL:
        case AV_OPT_TYPE_RATIONAL:
        case AV_OPT_TYPE_BINARY:
        case AV_OPT_TYPE_UINT64:
        case AV_OPT_TYPE_CONST:
            return option_format_t::numeric;
        break;

        case AV_OPT_TYPE_DOUBLE:
        case AV_OPT_TYPE_FLOAT:
            return option_format_t::real;
        break;

        case AV_OPT_TYPE_STRING:
        case AV_OPT_TYPE_IMAGE_SIZE:
        case AV_OPT_TYPE_PIXEL_FMT:
        case AV_OPT_TYPE_SAMPLE_FMT:
        case AV_OPT_TYPE_VIDEO_RATE:
        case AV_OPT_TYPE_COLOR:
        case AV_OPT_TYPE_CHANNEL_LAYOUT:
        case AV_OPT_TYPE_DURATION:
        case AV_OPT_TYPE_DICT:
            return option_format_t::string;
        break;
    }

    return option_format_t::unknown;
}

option_format_t device_option_t::option_format() const
{
    return option_format(type);
}

uint32_t audio_info_t::bps(sample_format_t sample_format)
{
    return av_get_bytes_per_sample(static_cast<AVSampleFormat>(sample_format));
}

std::size_t audio_info_t::sample_size(sample_format_t sample_format, uint32_t channels)
{
   return (bps(sample_format) * channels) / 8;
}

std::string audio_info_t::format_name(sample_format_t sample_format)
{
    return av_get_sample_fmt_name(static_cast<AVSampleFormat>(sample_format));
}

audio_info_t::audio_info_t(uint32_t sample_rate
                           , uint32_t channels
                           , sample_format_t sample_format)
    : sample_rate(sample_rate)
    , channels(channels)
    , sample_format(sample_format)
{

}

bool audio_info_t::operator ==(const audio_info_t &audio_info) const
{
    return sample_rate == audio_info.sample_rate
            && channels == audio_info.channels
            && sample_format == audio_info.sample_format;
}

bool audio_info_t::operator !=(const audio_info_t &audio_info) const
{
    return ! operator ==(audio_info);
}

uint32_t audio_info_t::bps() const
{
    return bps(sample_format);
}

std::size_t audio_info_t::sample_size() const
{
    return sample_size(sample_format, channels);
}

std::string audio_info_t::format_name() const
{
    return format_name(sample_format);
}

uint32_t video_info_t::bpp(pixel_format_t pixel_format)
{
    return av_get_bits_per_pixel(av_pix_fmt_desc_get(static_cast<AVPixelFormat>(pixel_format)));
}

std::size_t video_info_t::frame_size(pixel_format_t pixel_format
                                     , const frame_size_t &size
                                     , std::int32_t align)
{
    return av_image_get_buffer_size(static_cast<AVPixelFormat>(pixel_format)
                                    , size.width
                                    , size.height
                                    , align);
}

std::string video_info_t::format_name(pixel_format_t pixel_format)
{
    return av_get_pix_fmt_name(static_cast<AVPixelFormat>(pixel_format));
}

video_info_t::video_info_t(uint32_t width
                           , uint32_t height
                           , uint32_t fps
                           , pixel_format_t pixel_format)
    : video_info_t({ width, height}
                   , fps
                   , pixel_format)
{

}

video_info_t::video_info_t(frame_size_t size
                           , uint32_t fps
                           , pixel_format_t pixel_format)
    : size(size)
    , fps(fps)
    , pixel_format(pixel_format)
{

}

bool video_info_t::operator ==(const video_info_t &video_info) const
{
    return fps == video_info.fps
            && size == video_info.size
            && pixel_format == video_info.pixel_format;
}

bool video_info_t::operator !=(const video_info_t &video_info) const
{
    return ! operator ==(video_info);
}

uint32_t video_info_t::bpp() const
{
    return bpp(pixel_format);
}

std::size_t video_info_t::frame_size(std::int32_t align) const
{
    return frame_size(pixel_format
                      , size
                      , align);
}

std::string video_info_t::format_name() const
{
    return format_name(pixel_format);
}


media_info_t::media_info_t(const audio_info_t &audio_info)
    : audio_info(audio_info)
    , video_info()
{

}

media_info_t::media_info_t(const video_info_t &video_info)
    : video_info(video_info)
{

}

std::string frame_info_t::to_string() const
{
    std::stringstream ss;
    ss << "stream #" << stream_id << ":";

    switch(media_type)
    {
        case media_type_t::audio:
            ss << "A[" << media_info.audio_info.sample_rate
               << "/" << media_info.audio_info.bps()
               << "/" << media_info.audio_info.channels
               << "]";
        break;
        case media_type_t::video:
            ss << "V[" << media_info.video_info.size.width
               << "x" << media_info.video_info.size.height
               << "@" << media_info.video_info.fps
               << "]";
        break;
        default:
            ss << "D";
        break;
    }

    return ss.str();
}

std::string stream_info_t::to_string() const
{
    return frame_info_t::to_string().append(":c=" + codec_info.name);
}

frame_point_t::frame_point_t(uint32_t x, uint32_t y)
    : x(x)
    , y(y)
{

}

bool frame_point_t::is_null() const
{
    return x == 0 && y == 0;
}

bool frame_point_t::operator ==(const frame_point_t &frame_point) const
{
    return x == frame_point.x
            && y == frame_point.y;
}

bool frame_point_t::operator !=(const frame_point_t &frame_point) const
{
    return !operator==(frame_point);
}

frame_point_t &frame_point_t::operator +=(const frame_point_t &frame_point)
{
    x += frame_point.x;
    y += frame_point.y;

    return *this;
}

frame_point_t &frame_point_t::operator +=(const frame_size_t &frame_size)
{
    x += frame_size.width;
    y += frame_size.height;

    return *this;
}

frame_size_t::frame_size_t(uint32_t width, uint32_t height)
    : width(width)
    , height(height)
{

}

std::size_t frame_size_t::size() const
{
    return width * height;
}

bool frame_size_t::is_null() const
{
    return width == 0 || height == 0;
}

bool frame_size_t::operator ==(const frame_size_t &frame_size) const
{
    return width == frame_size.width
            && height == frame_size.height;
}

bool frame_size_t::operator !=(const frame_size_t &frame_size) const
{
    return !operator==(frame_size);
}

frame_size_t &frame_size_t::operator +=(const frame_size_t &frame_size)
{
    width += frame_size.width;
    height += frame_size.height;

    return *this;
}

frame_size_t &frame_size_t::operator +=(const frame_point_t &frame_point)
{
    width += frame_point.x;
    height += frame_point.y;

    return *this;
}

fragment_info_t::fragment_info_t(uint32_t x
                                 , uint32_t y
                                 , uint32_t width
                                 , uint32_t height
                                 , uint32_t frame_width
                                 , uint32_t frame_height
                                 , pixel_format_t pixel_format)
    : fragment_info_t({ x, y, width, height }
                      , { frame_width, frame_height }
                      , pixel_format)
{

}

fragment_info_t::fragment_info_t(const frame_rect_t& frame_rect
                                 , const frame_size_t& frame_size
                                 , pixel_format_t pixel_format)
    : frame_rect(frame_rect)
    , frame_size(frame_size)
    , pixel_format(pixel_format)
{

}

size_t fragment_info_t::get_fragment_size(std::int32_t align) const
{
    return video_info_t::frame_size(pixel_format, frame_rect.size, align);
}


size_t fragment_info_t::get_frame_size(std::int32_t align) const
{
    return video_info_t::frame_size(pixel_format, frame_size, align);
}

bool fragment_info_t::is_full() const
{
    return frame_rect.offset.is_null()
            && frame_rect.size == frame_size;
}

bool fragment_info_t::operator ==(const fragment_info_t &fragment_info) const
{
    return frame_rect == fragment_info.frame_rect
            && frame_size == fragment_info.frame_size
            && pixel_format == fragment_info.pixel_format;
}

bool fragment_info_t::operator !=(const fragment_info_t &fragment_info) const
{
    return !operator ==(fragment_info);
}

frame_rect_t::frame_rect_t(const frame_point_t &offset, const frame_size_t &size)
    : offset(offset)
    , size(size)
{

}

frame_rect_t::frame_rect_t(uint32_t x
                           , uint32_t y
                           , uint32_t width
                           , uint32_t height)
    : frame_rect_t({ x, y }
                   , { width, height })
{

}

bool frame_rect_t::operator ==(const frame_rect_t &frame_rect) const
{
    return offset == frame_rect.offset && size == frame_rect.size;
}

bool frame_rect_t::operator !=(const frame_rect_t &frame_rect) const
{
    return !operator==(frame_rect);
}

frame_rect_t &frame_rect_t::operator +=(const frame_size_t &frame_size)
{
    size += frame_size;
    return *this;
}

frame_rect_t &frame_rect_t::operator +=(const frame_point_t &frame_point)
{
    offset += frame_point;
    return *this;
}

bool frame_rect_t::is_join(const frame_size_t &frame_size) const
{
    return frame_size.width >= (offset.x + size.width)
            && frame_size.height >= (offset.y + size.height);
}

bool codec_info_t::is_coded() const
{
    return id > 0 && id != AV_CODEC_ID_RAWVIDEO;
}

}
