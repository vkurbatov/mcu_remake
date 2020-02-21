#include "libav_output_media_device.h"

#include "media/video/video_format.h"
#include "media/common/utils/format_converter.h"

namespace core
{

namespace media
{

static std::size_t fetch_libav_streams(const media_format_list_t& streams
                                       , ffmpeg::stream_info_list_t& libav_stream_list)
{
    for (const auto& s : streams)
    {
        ffmpeg::stream_info_t s_info;
        s_info.stream_id = s.stream_id;
        s_info.extra_data = s.extra_data;

        switch(s.media_type)
        {
            case media_type_t::video:
                s_info.media_info.media_type = ffmpeg::media_type_t::video;
                s_info.media_info.video_info.pixel_format = utils::format_conversion::to_ffmpeg_format(s.video_info().pixel_format);
                s_info.media_info.video_info.size = { s.video_info().size.width, s.video_info().size.height };
                s_info.media_info.video_info.fps = s.video_info().fps;
                s_info.codec_info.id = utils::format_conversion::to_ffmpeg_codec(s.video_info().pixel_format);
                s_info.codec_info.codec_params = ffmpeg::codec_params_t(s.parameters);
            break;
            case media_type_t::audio:
                s_info.media_info.media_type = ffmpeg::media_type_t::video;
                s_info.media_info.video_info.pixel_format = utils::format_conversion::to_ffmpeg_format(s.video_info().pixel_format);
                s_info.media_info.video_info.size = { s.video_info().size.width, s.video_info().size.height };
                s_info.media_info.video_info.fps = s.video_info().fps;
            break;
            case media_type_t::data:

            break;


        }
    }

    return libav_stream_list.size();
}

libav_output_media_device::libav_output_media_device(const media_format_list_t& media_streams)
    : m_media_streams(std::move(media_streams))
{

}

bool libav_output_media_device::set_media_streams(const media_format_list_t& media_streams)
{
    if (!m_libav_stream_publisher.is_opened())
    {
        m_media_streams = std::move(media_streams);
        return true;
    }

    return false;
}

bool libav_output_media_device::open(const std::string &uri)
{

}

bool libav_output_media_device::close()
{

}

bool libav_output_media_device::is_open() const
{

}

bool libav_output_media_device::is_established() const
{

}

media_format_list_t libav_output_media_device::streams() const
{
   return m_media_streams;
}

const control_parameter_list_t &libav_output_media_device::controls() const
{

}

bool libav_output_media_device::set_control(const std::string &control_name
                                            , const variant &control_value)
{

}

variant libav_output_media_device::get_control(const std::string &control_name
                                               , const variant &control_value) const
{

}

bool libav_output_media_device::on_frame(const i_media_frame &frame)
{

}

}

}
