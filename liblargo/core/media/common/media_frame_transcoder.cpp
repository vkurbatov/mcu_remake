#include "media_frame_transcoder.h"

#include "media/video/video_frame.h"
#include "media/audio/audio_frame.h"
#include "media/common/utils/format_converter.h"

namespace core
{

namespace media
{

static bool stream_info_from_format(const media_format_t& media_format
                                    , ffmpeg::stream_info_t& stream_info)
{
    switch(media_format.media_type)
    {
        case media_type_t::video:
        {
            const video::video_info_t& video_info = media_format.video_info();

            stream_info.stream_id = media_format.stream_id;
            stream_info.extra_data = media_format.extra_data;
            stream_info.codec_info.id = utils::format_conversion::to_ffmpeg_video_codec(video_info.pixel_format);
            stream_info.codec_info.name.clear();
            stream_info.media_info.media_type = ffmpeg::media_type_t::video;
            stream_info.media_info.video_info.fps = video_info.fps;
            stream_info.media_info.video_info.size = { video_info.size.width, video_info.size.height };
            stream_info.media_info.video_info.pixel_format = utils::format_conversion::to_ffmpeg_video_format(video_info.pixel_format);

            return true;
        }
        break;
        case media_type_t::audio:
        {
            const audio::audio_info_t& audio_info = media_format.audio_info();

            stream_info.stream_id = media_format.stream_id;
            stream_info.extra_data = media_format.extra_data;
            stream_info.codec_info.id = utils::format_conversion::to_ffmpeg_audio_codec(audio_info.sample_format);
            stream_info.codec_info.name.clear();
            stream_info.media_info.media_type = ffmpeg::media_type_t::audio;
            stream_info.media_info.audio_info.sample_rate = audio_info.sample_rate;
            stream_info.media_info.audio_info.channels = audio_info.channels;
            stream_info.media_info.audio_info.sample_format = utils::format_conversion::to_ffmpeg_audio_format(audio_info.sample_format);

            return true;
        }
        break;

    }
    return false;
}


media_frame_ptr_t libav_frame_to_media_frame(ffmpeg::frame_t& frame
                                             , const ffmpeg::stream_info_t& stream_info)
{
    media_frame_ptr_t result;

    if (!frame.media_data.empty())
    {
        auto buffer = media_buffer::create(std::move(frame.media_data));

        switch(stream_info.media_info.media_type)
        {
            case ffmpeg::media_type_t::video:
            {
                auto pixel_format = frame.info.is_encoded()
                        ? utils::format_conversion::from_ffmpeg_video_codec(frame.info.codec_id)
                        : utils::format_conversion::from_ffmpeg_video_format(frame.info.media_info.video_info.pixel_format);


                video::video_info_t video_info(pixel_format
                                               , { frame.info.media_info.video_info.size.width, frame.info.media_info.video_info.size.height }
                                               , frame.info.media_info.video_info.fps);

                media_format_t media_format(media_format_t(video_info
                                                           , stream_info.stream_id)
                                            );

                media_format.extra_data = stream_info.extra_data;

                result = video::video_frame::create(media_format
                                                    , buffer
                                                    , frame.info.id);

                if (result != nullptr && frame.info.key_frame)
                {
                    result->set_attributes(frame_attributes_t::key_frame);
                }
            }
            break;
            case ffmpeg::media_type_t::audio:
            {
                auto sample_format = frame.info.is_encoded()
                        ? utils::format_conversion::from_ffmpeg_audio_codec(frame.info.codec_id)
                        : utils::format_conversion::from_ffmpeg_audio_format(frame.info.media_info.audio_info.sample_format);


                audio::audio_info_t audio_info(sample_format
                                               , frame.info.media_info.audio_info.sample_rate
                                               , frame.info.media_info.audio_info.channels);

                media_format_t media_format(media_format_t(audio_info
                                                           , stream_info.stream_id)
                                            );

                media_format.extra_data = stream_info.extra_data;

                result = audio::audio_frame::create(media_format
                                                    , buffer
                                                    , frame.info.id);
            }
            break;
        }

    }

    return result;
}

media_frame_transcoder::media_frame_transcoder(const media_format_t& transcoding_format)
    : m_transcoding_format(transcoding_format)
{

}

const media_format_t& media_frame_transcoder::format() const
{
    return m_transcoding_format;
}

void media_frame_transcoder::reset()
{
    m_libav_transcoder.close();
}

bool media_frame_transcoder::setup(const media_format_t& transcoding_format)
{
    reset();

    m_transcoding_format = transcoding_format;

    return (m_transcoding_format.is_valid()
            && m_transcoding_format.is_encoded());
}

bool media_frame_transcoder::transcode(const i_media_frame &input_frame
                                       , media_frame_queue_t &frame_queue)
{
    if (m_transcoding_format.is_valid()
            && m_transcoding_format.is_encoded())
    {

        bool is_encoder =  !input_frame.media_format().is_encoded();

        auto transcode_type = is_encoder
                              ? ffmpeg::transcoder_type_t::encoder
                              : ffmpeg::transcoder_type_t::decoder;

        bool need_initialize = !m_libav_transcoder.is_open()
                || transcode_type != m_libav_transcoder.type();

        if (need_initialize)
        {
            const auto& target_format = /*is_encoder
                        ? input_frame.media_format()
                        : */m_transcoding_format;

            ffmpeg::stream_info_t stream_info {};

            if (stream_info_from_format(target_format
                                        , stream_info))
            {            
                m_libav_transcoder.open(stream_info
                                        , transcode_type
                                        , target_format.parameters);

            }
        }

        if (m_libav_transcoder.is_open())
        {
            ffmpeg::frame_queue_t libav_frame_queue;

            auto planes = input_frame.planes();

            if (m_libav_transcoder.transcode(planes[0]->data()
                                             , input_frame.size()
                                             , libav_frame_queue))
            {
                while (!libav_frame_queue.empty())
                {
                    auto& libav_frame = libav_frame_queue.front();

                    auto frame = libav_frame_to_media_frame(libav_frame
                                                            , m_libav_transcoder.config());
                    if (frame)
                    {
                        frame_queue.push(frame);
                    }

                    libav_frame_queue.pop();
                }

                return true;
            }
        }
    }

    return false;
}

}

}
