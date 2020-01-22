#ifndef FFMPEG_LIBAV_BASE_H
#define FFMPEG_LIBAV_BASE_H

#include "../base/frame_base.h"

#include <string>
#include <vector>
#include <queue>
#include <memory>
#include <functional>

namespace ffmpeg
{

typedef std::int32_t codec_id_t;
typedef std::int32_t pixel_format_t;
typedef std::int32_t sample_format_t;
const std::int32_t default_frame_align = 32;

const codec_id_t unknown_codec_id = -1;
const pixel_format_t unknown_pixel_format = -1;
const sample_format_t unknown_sample_format = -1;

extern const pixel_format_t pixel_format_none;
extern const pixel_format_t pixel_format_bgr24;
extern const pixel_format_t pixel_format_rgb24;
extern const pixel_format_t pixel_format_yuv420p;
extern const pixel_format_t pixel_format_yuv422p;

extern const pixel_format_t default_pixel_format;
extern const sample_format_t default_sample_format;

extern const codec_id_t codec_id_h264;
extern const codec_id_t codec_id_mjpeg;
extern const codec_id_t codec_id_raw_video;
extern const codec_id_t codec_id_none;
//extern const codec_id_t codec_id_yuv420p;

const std::size_t max_planes = 4;

typedef std::vector<std::uint8_t> media_data_t;

std::string error_to_string(std::int32_t av_error);

enum class media_type_t
{
    audio,
    video,
    data,
};

enum class streaming_event_t
{
    start,   
    stop,
    open,
    close
};

enum stream_mask_t : std::uint32_t
{
    stream_mask_empty = 0,
    stream_mask_audio = (1 << 0),
    stream_mask_video = (1 << 1),
    stream_mask_data =  (1 << 2),
    stream_mask_only_media = stream_mask_audio | stream_mask_video,
    stream_mask_all = stream_mask_only_media | stream_mask_data
};

struct codec_info_t
{
    codec_id_t                  id;
    std::string                 name;
    media_data_t                extra_data;
    bool is_coded() const;
};

typedef std::uint32_t option_type_t;

enum class option_format_t
{
    numeric,
    real,
    string,
    unknown
};

using frame_point_t = base::frame_point_t;
using frame_size_t = base::frame_size_t;
using frame_rect_t = base::frame_rect_t;

struct device_option_t
{
    std::string         name;
    std::string         help;
    std::int32_t        offset;
    option_type_t       type;

    struct
    {
        std::int64_t    numeric;
        double          real;
        std::string     string;
    }                   default_value;

    double              min;
    double              max;
    std::int32_t        flags;
    std::string         unit;

    static option_format_t option_format(option_type_t type);
    option_format_t option_format() const;
};

typedef std::vector<device_option_t> device_option_list_t;

struct audio_info_t
{
    std::uint32_t   sample_rate;
    std::uint32_t   channels;
    sample_format_t sample_format;

    static std::uint32_t bps(sample_format_t sample_format);
    static std::size_t sample_size(sample_format_t sample_format,  std::uint32_t channels);
    static std::string format_name(sample_format_t sample_format);

    audio_info_t(std::uint32_t sample_rate = 8000
                 , std::uint32_t channels = 1
                 , sample_format_t sample_format = default_sample_format);

    bool operator ==(const audio_info_t& audio_info) const;
    bool operator !=(const audio_info_t& audio_info) const;
    std::uint32_t bps() const;
    std::size_t sample_size() const;
    std::string format_name() const;
};

typedef std::vector<frame_size_t> plane_sizes_t;

struct plane_info_t
{
    void* data;
    frame_size_t size;
};

typedef std::vector<plane_info_t> plane_list_t;

struct video_info_t
{
    frame_size_t    size;
    std::uint32_t   fps;
    pixel_format_t  pixel_format;

    static std::uint32_t bpp(pixel_format_t pixel_format);
    static std::size_t frame_size(pixel_format_t pixel_format
                                  , const frame_size_t& size
                                  , std::int32_t align = default_frame_align);
    static std::string format_name(pixel_format_t pixel_format);
    static std::size_t planes(pixel_format_t pixel_format);
    static std::size_t plane_width(pixel_format_t pixel_format
                                   , std::uint32_t width
                                   , std::uint32_t plane_idx);
    static plane_sizes_t plane_sizes(pixel_format_t pixel_format
                                     , const frame_size_t& size
                                     , std::int32_t align = default_frame_align);

    static std::size_t split_slices(pixel_format_t pixel_format
                                     , const frame_size_t& size
                                     , void *slices[max_planes]
                                     , const void* data
                                     , std::int32_t align = default_frame_align);

    static plane_list_t split_planes(pixel_format_t pixel_format
                                     , const frame_size_t& size
                                     , const void* data
                                     , std::int32_t align = default_frame_align);

    static bool blackout(pixel_format_t pixel_format
                                , const frame_size_t& size
                                , void *slices[max_planes]);


    static bool is_planar(pixel_format_t pixel_format);


    video_info_t(std::uint32_t width
                 , std::uint32_t height
                 , std::uint32_t fps = 1
                 , pixel_format_t pixel_format = default_pixel_format);

    video_info_t(frame_size_t size = { 0, 0 }
                 , std::uint32_t fps = 1
                 , pixel_format_t pixel_format = default_pixel_format);

    bool operator ==(const video_info_t& video_info) const;
    bool operator !=(const video_info_t& video_info) const;
    std::uint32_t bpp() const;
    std::size_t frame_size(std::int32_t align = default_frame_align) const;
    std::string format_name() const;

    std::size_t planes() const;
    std::size_t plane_width(std::uint32_t plane_idx) const;
    plane_sizes_t plane_sizes() const;
    std::size_t split_slices(void *slices[max_planes]
                             , const void* data
                             , std::int32_t align = default_frame_align) const;
    plane_list_t split_planes(const void* data = nullptr
                             , std::int32_t align = default_frame_align);

    bool blackout(void *slices[max_planes]) const;
};

struct fragment_info_t
{
    frame_rect_t    frame_rect;
    frame_size_t    frame_size;
    pixel_format_t  pixel_format;

    fragment_info_t(std::uint32_t x
                    , std::uint32_t y
                    , std::uint32_t width
                    , std::uint32_t height
                    , std::uint32_t frame_width
                    , std::uint32_t frame_height
                    , pixel_format_t pixel_format = default_pixel_format);

    fragment_info_t(const frame_rect_t& frame_rect = { 0, 0, 0, 0 }
                    , const frame_size_t& frame_size = { 0, 0 }
                    , pixel_format_t pixel_format = default_pixel_format);

    bool operator ==(const fragment_info_t& fragment_info) const;
    bool operator !=(const fragment_info_t& fragment_info) const;

    std::size_t get_fragment_size(std::int32_t align = default_frame_align) const;
    std::size_t get_frame_size(std::int32_t align = default_frame_align) const;

    bool is_full() const;
    bool is_convertable() const;
};


struct media_info_t
{
    audio_info_t                audio_info;
    video_info_t                video_info;

    media_info_t() = default;
    media_info_t(const audio_info_t& audio_info);
    media_info_t(const video_info_t& video_info);
};

struct frame_info_t
{
    std::int32_t                stream_id;
    media_type_t                media_type;
    media_info_t                media_info;
    virtual std::string to_string() const;
};

struct frame_t
{
    frame_info_t    frame_info;
    media_data_t    frame_data;
};

struct stream_info_t: public frame_info_t
{
    codec_info_t                codec_info;
    std::string to_string() const override;
};

struct adaptive_timer_t
{
    std::uint64_t   time_base;
    adaptive_timer_t();

    static std::uint64_t now();

    void reset();
    bool wait(std::uint64_t wait_time
              , bool is_wait = true);
    std::uint64_t elapsed() const;
};

typedef std::vector<stream_info_t> stream_info_list_t;
typedef std::queue<media_data_t> media_queue_t;
typedef std::function<bool(const stream_info_t& stream_info
                           , media_data_t&& media_data)> stream_data_handler_t;

typedef std::function<void(const streaming_event_t& streaming_event)> stream_event_handler_t;

}

#endif // ffmpeg_LIBAV_BASE_H
