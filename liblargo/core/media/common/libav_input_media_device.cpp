#include "libav_input_media_device.h"
#include "media/video/video_frame.h"
#include "media/common/utils/format_converter.h"

namespace core
{

namespace media
{

media_format_ptr_t format_form_stream(const ffmpeg::stream_info_t& stream_info)
{

    auto pixel_format = stream_info.codec_info.is_coded()
            ? utils::format_conversion::from_ffmpeg_codec(stream_info.codec_info.id)
            : utils::format_conversion::from_ffmpeg_format(stream_info.media_info.video_info.pixel_format);

    if (stream_info.media_info.media_type == ffmpeg::media_type_t::video)
    {
        return video::video_format_t::create(pixel_format
                                             , { stream_info.media_info.video_info.size.width, stream_info.media_info.video_info.size.height}
                                             , stream_info.media_info.video_info.fps
                                             , stream_info.stream_id);
    }

    return nullptr;

}

static media_frame_ptr_t create_video_frame(const ffmpeg::stream_info_t& stream_info
                                            , ffmpeg::frame_t&& frame
                                            , frame_id_t frame_id)
{
    auto pixel_format = frame.info.codec_id > 0 && frame.info.codec_id != ffmpeg::codec_id_raw_video
            ? utils::format_conversion::from_ffmpeg_codec(frame.info.codec_id)
            : utils::format_conversion::from_ffmpeg_format(frame.info.media_info.video_info.pixel_format);

    video::video_format_t video_format(pixel_format
                                       , { frame.info.media_info.video_info.size.width, frame.info.media_info.video_info.size.height }
                                       , frame.info.media_info.video_info.fps
                                       , frame_id);

    video_format.extra_data = stream_info.extra_data;
    return video::video_frame::create(video_format
                                      , media_buffer::create(std::move(frame.media_data)));
}

libav_input_media_device::libav_input_media_device(i_media_sink& media_sink)
    : m_frame_counter(0)
    , m_libav_stream_capturer([&media_sink, this](const ffmpeg::stream_info_t& stream_info
                              , ffmpeg::frame_t&& libav_frame)
    {
        if (libav_frame.info.media_info.media_type == ffmpeg::media_type_t::video)
        {
           auto frame = create_video_frame(stream_info
                                           , std::move(libav_frame)
                                           , m_frame_counter);

           if (frame != nullptr)
           {
               m_frame_counter++;
               media_sink.on_frame(*frame);
           }
        }

        return true;
    })
{

}

bool libav_input_media_device::open(const std::string &uri)
{
    return m_libav_stream_capturer.open(uri);
}

bool libav_input_media_device::close()
{
    return m_libav_stream_capturer.close();
}

bool libav_input_media_device::is_open() const
{
    return m_libav_stream_capturer.is_opened();
}

bool libav_input_media_device::is_established() const
{
    return m_libav_stream_capturer.is_established();
}

media_format_list_t libav_input_media_device::streams() const
{
    media_format_list_t format_list;

    for (const auto& strm : m_libav_stream_capturer.streams())
    {
        auto format = format_form_stream(strm);
        if (format != nullptr)
        {
            format_list.emplace_back(std::move(format));
        }
    }

    return format_list;
}

const control_parameter_list_t& libav_input_media_device::controls() const
{
    static control_parameter_list_t control_parameter_list;
    return control_parameter_list;
}

bool libav_input_media_device::set_control(const std::string &control_name
                                           , const variant& control_value)
{
    return false;
}

variant libav_input_media_device::get_control(const std::string &control_name
                                              , const variant& default_value) const
{
    return variant();
}



}

}
