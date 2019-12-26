#include "libav_decoder.h"

#include <cstring>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
}

#define WBS_MODULE_NAME "ff:decoder"
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

}

static std::uint32_t dec_id = 0;

struct libav_decoder_context_t
{
    struct AVCodecContext*      context;
    struct AVFrame*             frame;
    struct AVPacket             packet;
    bool                        is_init;
    std::uint32_t               decoder_id;

    libav_decoder_context_t(codec_id_t codec_id
                        , const media_info_t& media_params
                        , const void *extra_data
                        , std::size_t extra_data_size)
        : context(nullptr)
        , frame(nullptr)
        , is_init(false)
        , decoder_id(0)
    {               
        decoder_id = dec_id++;

        LOG_D << "Create libav decoder context #" << decoder_id << " with codec_id = " << codec_id
              << " and extra data size = " << extra_data_size LOG_END;

        is_init = init(avcodec_find_decoder(static_cast<AVCodecID>(codec_id))
             , media_params
             , extra_data
             , extra_data_size);
    }

    libav_decoder_context_t(const std::string &decoder_name
                        , const media_info_t& media_params
                        , const void *extra_data
                        , std::size_t extra_data_size)
        : context(nullptr)
        , frame(nullptr)
        , is_init(false)
        , decoder_id(0)
    {
        decoder_id = dec_id++;

        LOG_D << "Decoder #" << decoder_id << ". Create libav decoder context with codec name = " << decoder_name
              << " and extra data size = " << extra_data_size LOG_END;

        is_init = init(avcodec_find_decoder_by_name(decoder_name.c_str())
             , media_params
             , extra_data
             , extra_data_size);        
    }

    bool init(const AVCodec* codec
              , const media_info_t& media_params
              , const void *extra_data
              , std::size_t extra_data_size)
    {

        if (codec != nullptr)
        {
            context = avcodec_alloc_context3(codec);

            if (extra_data != nullptr
                    && extra_data_size != 0)
            {
                context->extradata = static_cast<std::uint8_t*>(malloc(extra_data_size + AV_INPUT_BUFFER_PADDING_SIZE));
                std::memcpy(context->extradata, extra_data, extra_data_size);
                std::memset(context->extradata + extra_data_size, 0, AV_INPUT_BUFFER_PADDING_SIZE);
                context->extradata_size = extra_data_size;
            }

            if (context->codec_type == AVMEDIA_TYPE_AUDIO)
            {
                context->channels = media_params.audio_info.channels;
                context->channel_layout = context->channels == 1 ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO;
                context->sample_fmt = static_cast<AVSampleFormat>(media_params.audio_info.sample_format);
                context->sample_rate = media_params.audio_info.sample_rate;

                LOG_I << "Decoder #" << decoder_id << ". Initialize audio context [" <<  context->sample_rate
                      << "/16/" << context->channels << "]" LOG_END;
            }
            else
            {
                context->width = media_params.video_info.size.width;
                context->height = media_params.video_info.size.height;
                context->pix_fmt = static_cast<AVPixelFormat>(media_params.video_info.pixel_format);
                LOG_I << "Decoder #" << decoder_id << ". Initialize video context [" <<  media_params.video_info.size.width
                      << "x" << media_params.video_info.size.height << "@" << media_params.video_info.fps
                      << ":YUV420" LOG_END;
            }

            frame = av_frame_alloc();
            av_init_packet(&packet);

            auto result = avcodec_open2(context
                                        , context->codec
                                        , nullptr);

            if (result >= 0)
            {
                LOG_I << "Decoder #" << decoder_id << ". Codec " << context->codec->name << " initialized success" LOG_END;
            }
            else
            {
                LOG_E << "Decoder #" << decoder_id << ". Codec " << context->codec->name << " initialized failed, err = " << result LOG_END;
            }

            return result >= 0;
        }
        else
        {
            LOG_E << "Decoder #" << decoder_id << ". Context allocate error" LOG_END;
        }

        return false;
    }

    ~libav_decoder_context_t()
    {
        LOG_D << "Decoder #" << decoder_id << ". Destroy decoder" LOG_END;

        if (context != nullptr)
        {
            if (avcodec_is_open(context) > 0)
            {
                avcodec_close(context);
            }

            if (context->extradata != nullptr)
            {
                av_free(context->extradata);
                context->extradata = nullptr;
                context->extradata_size = 0;
            }

             avcodec_free_context(&context);
             LOG_I << "Decoder #" << decoder_id << ". Free context resource success" LOG_END;
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

        LOG_T << "Decoder #" << decoder_id << ". Fetch PCM16 audio frame with size " << audio_data.size() << " bytes" LOG_END;

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

        /*
        std::int32_t factor = frame->height;

        for (auto i = 0; i < AV_NUM_DATA_POINTERS && frame->linesize[i] > 0; i++)
        {
            frame_size += frame->linesize[i] * factor;
        }

        media_data_t video_data(frame_size);

        auto data_ptr = video_data.data();

        for (auto i = 0; i < AV_NUM_DATA_POINTERS && frame->linesize[i] > 0; i++)
        {
            std::memcpy(data_ptr, frame->data[i], frame->linesize[i] * factor);
            data_ptr += frame->linesize[i] * factor;
        }*/

        LOG_T << "Decoder #" << decoder_id << ". Fetch YUV420P video frame with size " << video_data.size() << " bytes" LOG_END;

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
                            s_info.media_info.video_info.fps = frame->sample_aspect_ratio.den;
                            s_info.media_info.video_info.pixel_format = frame->format;
                        }

                       frame_number++;

                       decoded_frame_t decoded_frame = { std::move(s_info)
                                                         , std::move(get_media_data()) };

                       decoded_frames.push( std::move(decoded_frame) );
                    }
                    else
                    {
                        LOG_W << "Decoder #" << decoder_id << " decode null size frame" LOG_END;
                    }
                }
                else if (result != AVERROR(EAGAIN)
                                           && result != AVERROR_EOF)
                {
                    LOG_E << "Decoder #" << decoder_id << ". Error call avcodec_receive_frame, err = " << result LOG_END;
                }

            }

        }
        else
        {
            LOG_E << "Decoder #" << decoder_id << ". Error avcodec_send_packet, err = " << result LOG_END;
        }

        return std::move(decoded_frames);
    }
};
//------------------------------------------------------------------------------
void libav_decoder_context_deleter_t::operator()(libav_decoder_context_t *libav_decoder_context_ptr)
{
    delete libav_decoder_context_ptr;
}
//------------------------------------------------------------------------------
libav_decoder::libav_decoder()
{
    LOG_T << "Create libav decoder" LOG_END;
}

bool libav_decoder::open(codec_id_t codec_id
                         , const media_info_t& media_params
                         , const void *extra_data
                         , std::size_t extra_data_size)
{
    LOG_D << "Open decoder by id " << codec_id LOG_END;
    m_decoder_context.reset(new libav_decoder_context_t(codec_id
                                                        , media_params
                                                        , extra_data
                                                        , extra_data_size));
    if (m_decoder_context->is_init == false)
    {
        m_decoder_context.reset();
    }

    return m_decoder_context != nullptr;
}

bool libav_decoder::open(const std::string &decoder_name
                         , const media_info_t& media_params
                         , const void *extra_data
                         , std::size_t extra_data_size)
{
    LOG_D << "Open decoder by name " << decoder_name LOG_END;
    m_decoder_context.reset(new libav_decoder_context_t(decoder_name
                                                        , media_params
                                                        , extra_data
                                                        , extra_data_size));

    if (m_decoder_context->is_init == false)
    {
        m_decoder_context.reset();
    }

    return m_decoder_context != nullptr;
}

bool libav_decoder::close()
{
    LOG_D << "Close decoder" LOG_END;
    m_decoder_context.reset();

    return true;
}

bool libav_decoder::is_open() const
{
    return m_decoder_context != nullptr;
}

decoded_queue_t libav_decoder::decode(const void *data
                                      , std::size_t size)
{
    decoded_queue_t result;

    LOG_D << "Decode packet size = " << size LOG_END;

    if (m_decoder_context != nullptr)
    {
        result = std::move(m_decoder_context->decode(data, size));
    }

    return std::move(result);
}

}
