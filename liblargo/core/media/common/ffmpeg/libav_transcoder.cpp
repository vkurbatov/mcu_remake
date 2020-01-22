#include "libav_transcoder.h"

#include <cstring>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
}

#define WBS_MODULE_NAME "ff:transcoder"
#include <core-tools/logging.h>


namespace ffmpeg
{


namespace utils
{

void float_to_pcm16(const void* float_buffer, void* pcm_buffer, std::size_t samples)
{
    auto float_ptr = reinterpret_cast<const float*>(float_buffer);
    auto pcm_ptr = reinterpret_cast<std::int16_t*>(pcm_buffer);

    while (samples-- > 0)
    {
        *pcm_ptr = *float_ptr * (1 << 15);

        pcm_ptr++;
        float_ptr++;
    }
}

void pcm16_to_float(const void* pcm_buffer, void* float_buffer, std::size_t samples)
{
    auto float_ptr = reinterpret_cast<float*>(float_buffer);
    auto pcm_ptr = reinterpret_cast<const std::int16_t*>(pcm_buffer);

    while (samples-- > 0)
    {
        *float_ptr = static_cast<double>(*pcm_ptr) / static_cast<double>(1 << 15);

        pcm_ptr++;
        float_ptr++;
    }
}

AVCodec* get_codec(const codec_info_t& codec_info)
{
    AVCodec* codec = nullptr;

    auto get_codec_by_name = codec_info.is_encoder
             ? &avcodec_find_encoder_by_name
             : &avcodec_find_decoder_by_name;

    auto get_codec_by_id = codec_info.is_encoder
             ? &avcodec_find_encoder
             : &avcodec_find_decoder;

    if (!codec_info.name.empty())
    {
        codec = get_codec_by_name(codec_info.name.c_str());
    }

    if (codec == nullptr)
    {
        codec = get_codec_by_id(static_cast<AVCodecID>(codec_info.id));
    }

    return codec;
}

}

static std::uint32_t g_transcoder_id = 0;

struct libav_transcoder_context_t
{
    struct AVCodecContext*      context;
    struct AVFrame*             frame;
    struct AVPacket             packet;
    bool                        is_init;
    stream_info_t               stream_info;
    std::uint32_t               transcoder_id;

    libav_transcoder_context_t(const stream_info_t& stream_info)
        : context(nullptr)
        , frame(nullptr)
        , is_init(false)
        , stream_info(stream_info)
        , transcoder_id(0)
    {
        transcoder_id = g_transcoder_id++;

        LOG_D << "Transcoder #" << transcoder_id << ". Create libav transcoder context with codec name = " << stream_info.codec_info.name
              << " and extra data size = " << stream_info.codec_info.extra_data.size() LOG_END;

        is_init = init();
    }

    bool init()
    {
        auto* codec = utils::get_codec(stream_info.codec_info);

        if (codec != nullptr)
        {
            context = avcodec_alloc_context3(codec);

            if (context != nullptr)
            {
                if (!stream_info.codec_info.extra_data.empty())
                {
                    context->extradata = stream_info.codec_info.extra_data.data();
                    context->extradata_size = stream_info.codec_info.extra_data.size();
                }

                if (context->codec_type == AVMEDIA_TYPE_AUDIO)
                {
                    context->channels = stream_info.media_info.audio_info.channels;
                    context->channel_layout = context->channels == 1 ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO;
                    context->sample_fmt = static_cast<AVSampleFormat>(stream_info.media_info.audio_info.sample_format);
                    context->sample_rate = stream_info.media_info.audio_info.sample_rate;

                    LOG_I << "Transcoder #" << transcoder_id << ". Initialize audio context [" <<  context->sample_rate
                          << "/16/" << context->channels << "]" LOG_END;
                }
                else
                {
                    context->width = stream_info.media_info.video_info.size.width;
                    context->height = stream_info.media_info.video_info.size.height;
                    context->pix_fmt = static_cast<AVPixelFormat>(stream_info.media_info.video_info.pixel_format);
                    context->framerate = av_d2q(stream_info.media_info.video_info.fps, 60);

                    LOG_I << "Transcoder #" << transcoder_id << ". Initialize video context [" <<  stream_info.media_info.video_info.size.width
                          << "x" << stream_info.media_info.video_info.size.height << "@" << stream_info.media_info.video_info.fps
                          << ":" << av_get_pix_fmt_name(context->pix_fmt) LOG_END;
                }

                frame = av_frame_alloc();
                av_init_packet(&packet);

                auto result = avcodec_open2(context
                                            , context->codec
                                            , nullptr);

                if (result >= 0)
                {
                    LOG_I << "Transcoder #" << transcoder_id << ". Codec " << context->codec->name << " initialized success" LOG_END;
                }
                else
                {
                    LOG_E << "Transcoder #" << transcoder_id << ". Codec " << context->codec->name << " initialized failed, err = " << result LOG_END;
                }

                return result >= 0;
            }

        }
        else
        {
            LOG_E << "Transcoder #" << transcoder_id << ". Context allocate error" LOG_END;
        }

        return false;
    }

    codec_id_t codec_id() const
    {
        return context != nullptr
                ? static_cast<codec_id_t>(context->codec_id)
                : unknown_codec_id;
    }

    ~libav_transcoder_context_t()
    {
        LOG_D << "Transcoder #" << transcoder_id << ". Destroy transcoder" LOG_END;

        if (context != nullptr)
        {
            if (avcodec_is_open(context) > 0)
            {
                avcodec_close(context);
            }

             context->extradata = nullptr;
             context->extradata_size = 0;

             avcodec_free_context(&context);
             LOG_I << "Transcoder #" << transcoder_id << ". Free context resource success" LOG_END;
        }

        if (frame != nullptr)
        {
            av_frame_free(&frame);
        }

    }

    media_data_t get_audio_data()
    {
        media_data_t audio_data;

        switch(frame->format)
        {
            case AV_SAMPLE_FMT_FLT:
            case AV_SAMPLE_FMT_FLTP:
            {
                audio_data.resize(frame->nb_samples * 2);
                utils::float_to_pcm16(frame->data[0], audio_data.data(), frame->nb_samples);
            }
            break;
            case AV_SAMPLE_FMT_S16:
            case AV_SAMPLE_FMT_S16P:
                audio_data = std::move(media_data_t(frame->data[0]
                        , frame->data[0] + frame->nb_samples * frame->channels * 2));

            break;
        }

        LOG_T << "Transcoder #" << transcoder_id << ". Fetch PCM16 audio frame with size " << audio_data.size() << " bytes" LOG_END;

        return audio_data;
    }

    media_data_t get_video_data(std::int32_t align = default_frame_align)
    {
        std::size_t frame_size = av_image_get_buffer_size(static_cast<AVPixelFormat>(frame->format)
                                                          , frame->width
                                                          , frame->height
                                                          , align);

        media_data_t video_data(frame_size);

        if (frame_size > 0)
        {
            av_image_copy_to_buffer(video_data.data()
                                    , video_data.size()
                                    , frame->data
                                    , frame->linesize
                                    , static_cast<AVPixelFormat>(frame->format)
                                    , frame->width
                                    , frame->height
                                    , align
                                    );
        }

        LOG_T << "Transcoder #" << transcoder_id << ". Fetch YUV420P video frame with size " << video_data.size() << " bytes" LOG_END;

        return std::move(video_data);
    }

    media_data_t get_media_data()
    {
        media_data_t media_data;

        switch (context->codec_type)
        {
            case AVMEDIA_TYPE_AUDIO:
                media_data = std::move(get_audio_data());
            break;
            case AVMEDIA_TYPE_VIDEO:
                media_data = std::move(get_video_data());
            break;
        }

        return std::move(media_data);
    }

    decoded_queue_t decode(const void* data
                        , std::size_t size)
    {
        decoded_queue_t decoded_frames;

        packet.data = const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(data));
        packet.size = size;

        std::int32_t frame_number = 0;

        auto result = avcodec_send_packet(context, &packet);

        if (result >= 0)
        {
            while (result >= 0)
            {
                result = avcodec_receive_frame(context, frame);

                if (result >= 0)
                {
                    if (frame->pkt_size > 0)
                    {
                        stream_info_t   s_info;

                        s_info.codec_info.id = context->codec_id;
                        s_info.codec_info.name = context->codec->name;
                        s_info.bitrate = context->bit_rate;
                        s_info.dts = frame->pkt_dts;
                        s_info.dts = frame->pkt_pts;
                        s_info.stream_id = frame_number;

                        if (context->codec->type == AVMEDIA_TYPE_AUDIO)
                        {
                            s_info.media_type = media_type_t::audio;
                            s_info.media_info.audio_info.sample_rate = frame->sample_rate;
                            s_info.media_info.audio_info.channels = frame->channels;
                            s_info.media_info.audio_info.sample_format = frame->format;
                        }
                        else
                        {
                            s_info.media_type = media_type_t::video;
                            s_info.media_info.video_info.size = { frame->width, frame->height };
                            s_info.media_info.video_info.fps = av_q2d(frame->sample_aspect_ratio);
                            s_info.media_info.video_info.pixel_format = frame->format;
                        }

                       frame_number++;

                       decoded_frame_t decoded_frame = { std::move(s_info)
                                                         , std::move(get_media_data()) };

                       decoded_frames.push( std::move(decoded_frame) );
                    }
                    else
                    {
                        LOG_W << "Transcoder #" << transcoder_id << " decode null size frame" LOG_END;
                    }
                }
                else if (result != AVERROR(EAGAIN)
                                           && result != AVERROR_EOF)
                {
                    LOG_E << "Transcoder #" << transcoder_id << ". Error call avcodec_receive_frame, err = " << result LOG_END;
                }

            }

        }
        else
        {
            LOG_E << "Transcoder #" << transcoder_id << ". Error avcodec_send_packet, err = " << result LOG_END;
        }

        return std::move(decoded_frames);
    }
};
//------------------------------------------------------------------------------
void libav_transcoder_context_deleter_t::operator()(libav_transcoder_context_t *libav_transcoder_context_ptr)
{
    delete libav_transcoder_context_ptr;
}
//------------------------------------------------------------------------------
libav_transcoder::libav_transcoder()
{
    LOG_T << "Create libav transcoder" LOG_END;
}

bool libav_transcoder::open(const stream_info_t &steam_info)
{
    LOG_D << "Open transcoder by id " << steam_info.codec_info.id LOG_END;
    m_transcoder_context.reset(new libav_transcoder_context_t(steam_info));

    if (m_transcoder_context->is_init == false)
    {
        m_transcoder_context.reset();
    }

    return m_transcoder_context != nullptr;
}

bool libav_transcoder::close()
{
    LOG_D << "Close transcoder" LOG_END;
    m_transcoder_context.reset();

    return true;
}

bool libav_transcoder::is_open() const
{
    return m_transcoder_context != nullptr;
}

decoded_queue_t libav_transcoder::decode(const void *data
                                      , std::size_t size)
{
    decoded_queue_t result;

    LOG_D << "Decode packet size = " << size LOG_END;

    if (m_transcoder_context != nullptr)
    {
        result = std::move(m_transcoder_context->decode(data, size));
    }

    return std::move(result);
}

codec_id_t libav_transcoder::codec_id() const
{
    return m_transcoder_context != nullptr
            ? m_transcoder_context->codec_id()
            : unknown_codec_id;
}

}
