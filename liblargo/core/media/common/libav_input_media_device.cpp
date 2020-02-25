#include "libav_input_media_device.h"
#include "media/video/video_frame.h"
#include "media/audio/audio_frame.h"
#include "media/common/codec_params.h"
#include "media/common/utils/format_converter.h"

namespace core
{

namespace media
{

bool format_form_stream(const ffmpeg::stream_info_t& stream_info
                        , media_format_t& media_format)
{

    switch(stream_info.media_info.media_type)
    {
        case ffmpeg::media_type_t::video:
        {
            auto pixel_format = stream_info.codec_info.is_coded()
                ? utils::format_conversion::from_ffmpeg_video_codec(stream_info.codec_info.id)
                : utils::format_conversion::from_ffmpeg_video_format(stream_info.media_info.video_info.pixel_format);

            video::video_info_t video_info(pixel_format
                                           , { stream_info.media_info.video_info.size.width, stream_info.media_info.video_info.size.height}
                                           , stream_info.media_info.video_info.fps);

            media_format = media_format_t(video_info
                                          , stream_info.stream_id);

            return true;
        }
        break;
        case ffmpeg::media_type_t::audio:
        {
            auto sample_format = stream_info.codec_info.is_coded()
                ? utils::format_conversion::from_ffmpeg_audio_codec(stream_info.codec_info.id)
                : utils::format_conversion::from_ffmpeg_audio_format(stream_info.media_info.audio_info.sample_format);

            audio::audio_info_t audio_info(sample_format
                                           , stream_info.media_info.audio_info.sample_rate
                                           , stream_info.media_info.audio_info.channels);

            media_format = media_format_t(audio_info
                                          , stream_info.stream_id);

        return true;
        }
        break;
    }

    return false;

}

static media_frame_ptr_t create_video_frame(const ffmpeg::stream_info_t& stream_info
                                            , ffmpeg::frame_t&& frame
                                            , frame_id_t frame_id)
{
    auto pixel_format = frame.info.codec_id > 0 && frame.info.codec_id != ffmpeg::codec_id_raw_video
            ? utils::format_conversion::from_ffmpeg_video_codec(frame.info.codec_id)
            : utils::format_conversion::from_ffmpeg_video_format(frame.info.media_info.video_info.pixel_format);


    video::video_info_t video_info(pixel_format
                                   , { frame.info.media_info.video_info.size.width, frame.info.media_info.video_info.size.height}
                                   , frame.info.media_info.video_info.fps);

    media_format_t media_format(video_info
                                , stream_info.stream_id);


    media_format.codec_params().extra_data = stream_info.extra_data;
    return video::video_frame::create(media_format
                                      , media_buffer::create(std::move(frame.media_data))
                                      , frame_id);
}

static media_frame_ptr_t create_audio_frame(const ffmpeg::stream_info_t& stream_info
                                            , ffmpeg::frame_t&& frame
                                            , frame_id_t frame_id)
{
    auto sample_format = frame.info.codec_id > 0
            ? utils::format_conversion::from_ffmpeg_audio_codec(frame.info.codec_id)
            : utils::format_conversion::from_ffmpeg_audio_format(frame.info.media_info.video_info.pixel_format);


    audio::audio_info_t audio_info(sample_format
                                   , stream_info.media_info.audio_info.sample_rate
                                   , stream_info.media_info.audio_info.channels);

    media_format_t media_format(audio_info
                                , stream_info.stream_id);


    media_format.codec_params().extra_data = stream_info.extra_data;
    return audio::audio_frame::create(media_format
                                      , media_buffer::create(std::move(frame.media_data))
                                      , frame_id);
}

static media_frame_ptr_t create_media_frame(const ffmpeg::stream_info_t& stream_info
                                              , ffmpeg::frame_t&& frame
                                              , frame_id_t& video_frame_id
                                              , frame_id_t& audio_frame_id)
{
    switch(stream_info.media_info.media_type)
    {
        case ffmpeg::media_type_t::video:
            return create_video_frame(stream_info
                                      , std::move(frame)
                                      , video_frame_id++);
        break;
        case ffmpeg::media_type_t::audio:
            return create_audio_frame(stream_info
                                      , std::move(frame)
                                      , audio_frame_id++);
        break;
    }

    return nullptr;
}

libav_input_media_device::libav_input_media_device(i_media_sink& media_sink)
    : m_video_frame_counter(0)
    , m_audio_frame_counter(0)
    , m_libav_stream_capturer([&media_sink, this](const ffmpeg::stream_info_t& stream_info
                              , ffmpeg::frame_t&& libav_frame)
    {

        auto frame = create_media_frame(stream_info
                                        , std::move(libav_frame)
                                        , m_video_frame_counter
                                        , m_audio_frame_counter);

       if (frame != nullptr)
       {
           media_sink.on_frame(*frame);
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
        media_format_t media_format;
        if (format_form_stream(strm, media_format))
        {
            format_list.emplace_back(std::move(media_format));
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
