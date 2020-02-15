#include "libav_input_media_device.h"
#include "media/video/video_frame.h"
#include "media/common/utils/format_converter.h"

namespace core
{

namespace media
{

static media_frame_ptr_t create_video_frame(const ffmpeg::stream_info_t& stream_info
                                            , ffmpeg::frame_t&& frame)
{
    auto pixel_format = frame.info.codec_id > 0 && frame.info.codec_id != ffmpeg::codec_id_raw_video
            ? utils::format_conversion::from_ffmpeg_codec(frame.info.codec_id)
            : utils::format_conversion::from_ffmpeg_format(frame.info.media_info.video_info.pixel_format);

    video::video_format_t video_format(pixel_format
                                       , { frame.info.media_info.video_info.size.width, frame.info.media_info.video_info.size.height }
                                       , frame.info.media_info.video_info.fps);

    video_format.extra_data = stream_info.extra_data;
    return video::video_frame::create(video_format
                                      , media_buffer::create(std::move(frame.media_data))
                                      , frame.info.id);
}

libav_input_media_device::libav_input_media_device(i_media_sink& media_sink)
{
    auto frame_handler = [&media_sink](const ffmpeg::stream_info_t& stream_info
                                       , ffmpeg::frame_t&& libav_frame) ->
    bool
    {       
        if (libav_frame.info.media_info.media_type == ffmpeg::media_type_t::video)
        {
            auto frame = create_video_frame(stream_info
                                            , std::move(libav_frame));

            if (frame != nullptr)
            {
                media_sink.on_frame(*frame);
            }
        }

        return true;
    };

    m_libav_stream_capturer.reset(new ffmpeg::libav_stream_capturer(frame_handler));
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

const control_parameter_list_t& libav_input_media_device::controls() const
{
    static control_parameter_list_t control_parameter_list;
    return control_parameter_list;
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
