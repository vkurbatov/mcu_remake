#ifndef FFMPEG_WRAPPER_LIBAV_BASE_H
#define FFMPEG_WRAPPER_LIBAV_BASE_H

#include <string>
#include <vector>
#include <queue>
#include <memory>
#include <functional>

namespace ffmpeg_wrapper
{

typedef std::int32_t codec_id_t;
typedef std::int32_t pixel_format_t;
typedef std::int32_t sample_format_t;


const pixel_format_t unknown_pixel_format = -1;
const sample_format_t unknown_sample_format = -1;

extern const pixel_format_t pixel_format_bgr24;
extern const pixel_format_t pixel_format_rgb24;

extern const pixel_format_t default_pixel_format;
extern const sample_format_t default_sample_format;

extern const codec_id_t codec_id_h264;
//extern const codec_id_t codec_id_yuv420p;

typedef std::vector<std::uint8_t> media_data_t;

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
};

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

struct frame_size_t; //fwd

struct frame_point_t
{
    std::uint32_t   x;
    std::uint32_t   y;

    frame_point_t(std::uint32_t x = 0
                  , std::uint32_t y = 0);

    bool operator ==(const frame_point_t& frame_point) const;
    bool operator !=(const frame_point_t& frame_point) const;

    frame_point_t& operator +=(const frame_point_t& frame_point);
    frame_point_t& operator +=(const frame_size_t& frame_size);
};

struct frame_size_t
{
    std::uint32_t   width;
    std::uint32_t   height;

    frame_size_t(std::uint32_t width = 0
                  , std::uint32_t height = 0);

    std::size_t size() const;
    bool is_null() const;
    bool operator ==(const frame_size_t& frame_size) const;
    bool operator !=(const frame_size_t& frame_size) const;

    frame_size_t& operator +=(const frame_size_t& frame_size);
    frame_size_t& operator +=(const frame_point_t& frame_point);

};

struct frame_rect_t
{
    frame_point_t   point;
    frame_size_t   size;

    frame_rect_t(const frame_point_t& point = { 0, 0 }
            , const frame_size_t& size = { 0, 0 });

    frame_rect_t(std::uint32_t x
                 , std::uint32_t y
                 , std::uint32_t width
                 , std::uint32_t height);

    bool operator ==(const frame_rect_t& frame_rect) const;
    bool operator !=(const frame_rect_t& frame_rect) const;

    frame_rect_t& operator +=(const frame_size_t& frame_size);
    frame_rect_t& operator +=(const frame_point_t& frame_point);

    bool is_join(const frame_size_t& frame_size) const;

};

struct video_info_t
{
    frame_size_t    size;
    std::uint32_t   fps;
    pixel_format_t  pixel_format;

    static std::uint32_t bpp(pixel_format_t pixel_format);
    static std::size_t frame_size(pixel_format_t pixel_format
                                  , const frame_size_t& size
                                  , std::int32_t align = 0);
    static std::string format_name(pixel_format_t pixel_format);

    video_info_t(std::uint32_t width
                 , std::uint32_t height
                 , std::uint32_t fps = 1
                 , pixel_format_t pixel_format = default_pixel_format);

    video_info_t(frame_size_t size = { 0, 0}
                 , std::uint32_t fps = 1
                 , pixel_format_t pixel_format = default_pixel_format);

    bool operator ==(const video_info_t& video_info) const;
    bool operator !=(const video_info_t& video_info) const;
    std::uint32_t bpp() const;
    std::size_t frame_size(std::int32_t align = 0) const;
    std::string format_name() const;
};

struct fragment_info_t
{
    frame_point_t   offset;
    frame_size_t    size;
    pixel_format_t  pixel_format;

    fragment_info_t(std::uint32_t x
                          , std::uint32_t y
                          , std::uint32_t width
                          , std::uint32_t height
                          , pixel_format_t pixel_format = default_pixel_format);

    fragment_info_t(const frame_point_t& offset = { 0, 0 }
                          , const frame_size_t& size = { 0, 0 }
                          , pixel_format_t pixel_format = default_pixel_format);


    bool operator ==(const fragment_info_t& fragment_info) const;
    bool operator !=(const fragment_info_t& fragment_info) const;
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

struct stream_info_t: public frame_info_t
{
    codec_info_t                codec_info;
    std::string to_string() const override;
};

typedef std::vector<stream_info_t> stream_info_list_t;
typedef std::queue<media_data_t> media_queue_t;
typedef std::function<bool(const stream_info_t& stream_info
                           , media_data_t&& media_data)> stream_data_handler_t;

typedef std::function<void(const streaming_event_t& streaming_event)> stream_event_handler_t;

}

#endif // FFMPEG_WRAPPER_LIBAV_BASE_H
