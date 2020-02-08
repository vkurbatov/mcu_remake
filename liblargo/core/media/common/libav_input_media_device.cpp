#include "libav_input_media_device.h"
#include "media/video/video_frame.h"
#include "media/common/utils/format_converter.h"

namespace core
{

namespace media
{

static media_frame_ptr_t create_video_frame(const ffmpeg::stream_info_t& stream_info
                                            , ffmpeg::media_data_t&& media_data)
{
    auto pixel_format = stream_info.codec_info.is_coded()
            ? utils::format_conversion::from_ffmpeg_codec(stream_info.codec_info.id)
            : utils::format_conversion::from_ffmpeg_format(stream_info.media_info.video_info.pixel_format);

    video::video_format_t video_format(pixel_format
                                       , { stream_info.media_info.video_info.size.width, stream_info.media_info.video_info.size.height }
                                       , stream_info.media_info.video_info.fps);

    return video::video_frame::create(video_format
                                      , media_buffer::create(std::move(media_data)));
}

libav_input_media_device::libav_input_media_device(i_media_sink &media_sink)
{
    // bool(const stream_info_t& stream_info
    // , media_data_t&& media_data

    auto data_handler = [&media_sink](const ffmpeg::stream_info_t& stream_info
                                                 , ffmpeg::media_data_t&& media_data) ->
    bool
    {
        if (stream_info.media_info.media_type == ffmpeg::media_type_t::video)
        {
            auto frame = create_video_frame(stream_info
                                            , std::move(media_data));

            if (frame != nullptr)
            {
                media_sink.on_frame(*frame);
            }
        }

        return true;
    };

    m_libav_stream_capturer.reset(new ffmpeg::libav_stream_capturer(data_handler));
}

bool libav_input_media_device::open(const std::string &uri)
{
    return m_libav_stream_capturer->open(uri);
}

bool libav_input_media_device::close()
{
    return m_libav_stream_capturer->close();
}

bool libav_input_media_device::is_open() const
{
    return m_libav_stream_capturer->is_opened();
}

bool libav_input_media_device::is_established() const
{
    return m_libav_stream_capturer->is_established();
}

control_parameter_list_t libav_input_media_device::controls() const
{
    return control_parameter_list_t();
}

bool libav_input_media_device::set_control(const std::string &control_name
                                           , const variant control_value)
{
    return false;
}

variant libav_input_media_device::get_control(const std::string &control_name
                                              , const variant control_value) const
{
    return variant();
}



}

}
