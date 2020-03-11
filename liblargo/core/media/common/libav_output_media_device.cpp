#include "libav_output_media_device.h"

//#include "media/video/video_format.h"
#include "media/common/utils/format_converter.h"
#include "media/common/i_media_frame.h"
#include "media/common/codec_params.h"

namespace core
{

namespace media
{

static bool stream_form_format(const media_format_t& format
                               , ffmpeg::stream_info_t& s_info)
{
    switch(format.media_type)
    {
        case media_type_t::video:
        {
            const auto& video_info = format.video_info();
            s_info.media_info.media_type = ffmpeg::media_type_t::video;
            s_info.media_info.video_info.pixel_format = utils::format_conversion::to_ffmpeg_video_format(video_info.pixel_format);
            s_info.media_info.video_info.size = { video_info.size.width, video_info.size.height };
            s_info.media_info.video_info.fps = video_info.fps;
            s_info.codec_info.id = utils::format_conversion::to_ffmpeg_video_codec(video_info.pixel_format);

        }
        break;
        case media_type_t::audio:
        {
            const auto& audio_info = format.audio_info();
            s_info.media_info.media_type = ffmpeg::media_type_t::audio;
            s_info.media_info.audio_info.sample_format = utils::format_conversion::to_ffmpeg_audio_format(audio_info.sample_format);
            s_info.media_info.audio_info.sample_rate = audio_info.sample_rate;
            s_info.media_info.audio_info.sample_format = audio_info.channels;
            s_info.codec_info.id = utils::format_conversion::to_ffmpeg_audio_codec(audio_info.sample_format);

        }
        break;
        default:
            return false;
        break;
    }

    s_info.stream_id = format.stream_id;
    s_info.extra_data = format.codec_params().extra_data;
    s_info.codec_info.codec_params.bitrate = format.codec_params().bitrate;
    s_info.codec_info.codec_params.frame_size = format.codec_params().frame_size;
    s_info.codec_info.codec_params.gop = format.codec_params().gop_size;
    s_info.codec_info.codec_params.set_global_header(format.codec_params().global_header);

    return true;
}

static std::size_t fetch_libav_streams(const media_format_list_t& streams
                                       , ffmpeg::stream_info_list_t& libav_stream_list)
{
    for (const auto& s : streams)
    {
        ffmpeg::stream_info_t s_info;

        if (stream_form_format(s
                               , s_info))
        {
            libav_stream_list.emplace_back(std::move(s_info));
        }
    }

    return libav_stream_list.size();
}

libav_output_media_device::libav_output_media_device(const media_format_list_t& media_streams)
{
    set_media_streams(media_streams);
}

bool libav_output_media_device::set_media_streams(const media_format_list_t& media_streams)
{
    if (!m_libav_stream_publisher.is_opened())
    {
        m_media_streams.clear();
        for (const auto& s : media_streams)
        {
            m_media_streams.emplace(s.stream_id
                                    , s);
        }
        return true;
    }

    return false;
}

bool libav_output_media_device::open(const std::string &uri)
{
    ffmpeg::stream_info_list_t libav_stream_list;
    fetch_libav_streams(streams()
                        , libav_stream_list);

    if (!libav_stream_list.empty())
    {
        return m_libav_stream_publisher.open(uri
                                             , libav_stream_list);
    }

    return false;
}

bool libav_output_media_device::close()
{
    return m_libav_stream_publisher.close();
}

bool libav_output_media_device::is_open() const
{
    return m_libav_stream_publisher.is_opened();
}

bool libav_output_media_device::is_established() const
{
    return m_libav_stream_publisher.is_established();
}

media_format_list_t libav_output_media_device::streams() const
{
    media_format_list_t format_list;
    for (const auto& s : m_media_streams)
    {
        format_list.push_back(s.second);
    }
    return format_list;
}

const control_parameter_list_t &libav_output_media_device::controls() const
{
    return control_parameter_list_t();
}

bool libav_output_media_device::set_control(const std::string &control_name
                                            , const variant &control_value)
{
    return false;
}

variant libav_output_media_device::get_control(const std::string &control_name
                                               , const variant &default_value) const
{
    return default_value;
}

bool libav_output_media_device::on_frame(media_frame_ptr_t frame)
{
    if (m_libav_stream_publisher.is_opened() && frame != nullptr)
    {
        auto it = m_media_streams.find(frame->media_format().stream_id);

        if (it != m_media_streams.end())
        {
            if (it->second.media_type == frame->media_format().media_type)
            {
                ffmpeg::stream_info_t s_info;
                if (stream_form_format(frame->media_format()
                                       , s_info))
                {
                    bool key_frame = static_cast<std::uint32_t>(frame->frame_attributes()) & static_cast<std::uint32_t>(frame_attributes_t::key_frame)
                            != static_cast<std::uint32_t>(frame_attributes_t::undefined);
                    return m_libav_stream_publisher.push_frame(s_info.stream_id
                                                               , frame->planes()[0]->data()
                                                               , frame->size()
                                                               , key_frame);
                }
            }
        }
    }

    return false;
}

}

}
