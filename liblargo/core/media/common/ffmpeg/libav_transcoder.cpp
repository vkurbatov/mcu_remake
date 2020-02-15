#include "libav_transcoder.h"
#include "libav_utils.h"

#include <cstring>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
}

#define WBS_MODULE_NAME "ff:transcoder"
#include <core-tools/logging.h>

#include <map>

namespace ffmpeg
{

namespace utils
{

void float_to_pcm16(const void* float_buffer
                    , void* pcm_buffer
                    , std::size_t samples)
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

void pcm16_to_float(const void* pcm_buffer
                    , void* float_buffer
                    , std::size_t samples)
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

void update_context_info(const AVCodecContext& av_context
                         , stream_info_t& stream_info
                         , AVFrame& av_frame)
{

    stream_info.codec_info.id = av_context.codec->id;
    stream_info.codec_info.name = av_context.codec->name;
    stream_info.codec_info.codec_params.bitrate = av_context.bit_rate;
    stream_info.codec_info.codec_params.frame_size = av_context.frame_size;
    stream_info.codec_info.codec_params.flags1 = av_context.flags;
    stream_info.codec_info.codec_params.flags2 = av_context.flags2;

    stream_info.media_info << av_context;

    if (av_context.codec_type == AVMEDIA_TYPE_AUDIO)
    {
        av_frame.channels = av_context.channels;
        av_frame.channel_layout = av_context.channel_layout;
        av_frame.format = av_context.sample_fmt;
    }
    else
    {

        stream_info.codec_info.codec_params.gop = av_context.gop_size;

        av_frame.width = av_context.width;
        av_frame.height = av_context.height;
        av_frame.sample_aspect_ratio = av_context.time_base;
        av_frame.format = av_context.pix_fmt;
    }

    if (av_context.extradata != nullptr
            && av_context.extradata_size > 0)
    {
        stream_info.extra_data.resize(av_context.extradata_size + AV_INPUT_BUFFER_PADDING_SIZE, 0);

        std::memcpy(stream_info.extra_data.data()
                    , av_context.extradata
                    , av_context.extradata_size);
    }

    av_frame.sample_rate = av_context.sample_rate;
    av_frame.pts = AV_NOPTS_VALUE;
}

bool set_custom_option(AVCodecContext& av_context
                          , const option_t& option)
{
    enum class custom_option_t
    {
        thread_count,
        bitrate,
        gop,
        frame_size,
        global_header
    };

    static std::map<std::string, custom_option_t> custom_option_table =
    {
        { "libav_thread_count", custom_option_t::thread_count },
        { "libav_bitrate", custom_option_t::bitrate },
        { "libav_gop", custom_option_t::gop },
        { "libav_frame_size", custom_option_t::frame_size },
        { "libav_global_header", custom_option_t::global_header }
    };

    auto it = custom_option_table.find(option.first);

    if (it != custom_option_table.end())
    {
        switch(it->second)
        {
            case custom_option_t::thread_count:
                av_context.thread_count = std::atoi(option.second.c_str());
            break;
            case custom_option_t::bitrate:
                av_context.bit_rate = std::atoi(option.second.c_str());
            break;
            case custom_option_t::gop:
                av_context.gop_size = std::atoi(option.second.c_str());
            break;
            case custom_option_t::frame_size:
                av_context.frame_size = std::atoi(option.second.c_str());
            break;
            case custom_option_t::global_header:
                if (option.second.empty() || std::atoi(option.second.c_str()) != 0)
                {
                    av_context.flags |= CODEC_FLAG_GLOBAL_HEADER;
                }
                else
                {
                    av_context.flags &= ~CODEC_FLAG_GLOBAL_HEADER;
                }
            break;
        }
        return true;
    }

    return false;
}

void set_extended_options(AVCodecContext& av_context
                          , AVDictionary *av_options
                          , const std::string& options)
{
    for (const auto& opt : parse_option_list(options))
    {
        if (!set_custom_option(av_context
                               , opt))
        {
            av_dict_set(&av_options, opt.first.c_str(), opt.second.c_str(), 0);
        }
    }
}

AVCodec* get_codec(const codec_info_t& codec_info
                   , bool is_encoder)
{
    AVCodec* codec = nullptr;

    auto get_codec_by_name = is_encoder
             ? &avcodec_find_encoder_by_name
             : &avcodec_find_decoder_by_name;

    auto get_codec_by_id = is_encoder
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

static bool operator & (transcode_flag_t lfl, transcode_flag_t rfl)
{
    return static_cast<std::uint32_t>(lfl) & static_cast<std::uint32_t>(rfl) != 0;
}


static std::uint32_t g_context_id = 0;

struct libav_codec_context_t
{
    struct AVCodecContext*      av_context;
    struct AVFrame              av_frame;
    struct AVPacket             av_packet;
    media_data_t                resample_buffer;
    std::uint32_t               context_id;
    std::int32_t                frame_counter;
    bool                        is_encoder;
    bool                        is_init;

    libav_codec_context_t(stream_info_t& stream_info
                          , bool is_encoder
                          , const std::string& options)
        : av_context(nullptr)
        , av_frame{}
        , av_packet{}
        , context_id(++g_context_id)
        , frame_counter(0)
        , is_encoder(is_encoder)
        , is_init(false)
    {
        is_init = init(stream_info
                       , options);
    }

    ~libav_codec_context_t()
    {
        LOG_D << "Transcoder #" << context_id << ". Destroy transcoder" LOG_END;

        reset();
    }

    void reset()
    {
        is_init = false;

        if (av_context != nullptr)
        {
            if (avcodec_is_open(av_context) > 0)
            {
                avcodec_close(av_context);
            }

             av_context->extradata = nullptr;
             av_context->extradata_size = 0;

             avcodec_free_context(&av_context);
             LOG_I << "Transcoder #" << context_id << ". Free context resource success" LOG_END;


             av_context = nullptr;
             av_frame = {};
             av_packet = {};
        }
    }

    bool reinit(stream_info_t& stream_info
                , bool is_encoder
                , const std::string& options)
    {
        reset();
        this->is_encoder = is_encoder;
        is_init = init(stream_info
                       , options);
    }

    bool init(stream_info_t& stream_info
              , const std::string& options)
    {
        auto* codec = utils::get_codec(stream_info.codec_info
                                       , is_encoder);

        if (codec != nullptr)
        {
            av_context = avcodec_alloc_context3(codec);

            if (av_context != nullptr)
            {
                if (!stream_info.extra_data.empty())
                {
                    av_context->extradata = stream_info.extra_data.data();
                    av_context->extradata_size = stream_info.extra_data.size();
                }

                if (av_context->codec_type == AVMEDIA_TYPE_AUDIO)
                {
                    stream_info.media_info >> *(av_context);

                    LOG_I << "Transcoder #" << context_id << ". Initialize audio context [" <<  av_context->sample_rate
                          << "/16/" << av_context->channels << "]" LOG_END;
                }
                else
                {
                    stream_info.media_info >> *(av_context);

                    switch(av_context->codec_id)
                    {
                        case AV_CODEC_ID_MPEG2VIDEO:
                            av_context->max_b_frames = 2;
                        break;
                        case AV_CODEC_ID_MPEG1VIDEO:
                            av_context->mb_decision = 2;
                        break;
                    }

                    LOG_I << "Transcoder #" << context_id << ". Initialize video context [" <<  stream_info.media_info.video_info.size.width
                          << "x" << stream_info.media_info.video_info.size.height << "@" << stream_info.media_info.video_info.fps
                          << ":" << av_get_pix_fmt_name(av_context->pix_fmt) << "]" LOG_END;
                }

                utils::merge_codec_params(*av_context
                                           , stream_info.codec_info.codec_params);

                av_init_packet(&av_packet);

                AVDictionary *av_options = nullptr;

                utils::set_extended_options(*av_context
                                            , av_options
                                            , options);


                auto result = avcodec_open2(av_context
                                            , av_context->codec
                                            , &av_options);

                if (result >= 0)
                {
                    utils::update_context_info(*av_context
                                               , stream_info
                                               , av_frame);

                    LOG_I << "Transcoder #" << context_id << ". Codec " << stream_info.codec_info.to_string() << " initialized success" LOG_END;
                }
                else
                {
                    LOG_E << "Transcoder #" << context_id << ". Codec " << stream_info.codec_info.to_string()
                          << " initialized failed, error = " << result
                          << ": " <<  error_to_string(result) LOG_END;
                }

                return result >= 0;
            }

        }
        else
        {
            LOG_E << "Transcoder #" << context_id << ". Context allocate error" LOG_END;
        }

        return false;
    }

    media_data_t get_audio_data()
    {
        media_data_t audio_data;

        switch(av_frame.format)
        {
            case AV_SAMPLE_FMT_FLT:
            case AV_SAMPLE_FMT_FLTP:
            {
                audio_data.resize(av_frame.nb_samples * 2);
                utils::float_to_pcm16(av_frame.data[0], audio_data.data(), av_frame.nb_samples);
            }
            break;
            case AV_SAMPLE_FMT_S16:
            case AV_SAMPLE_FMT_S16P:
                audio_data = std::move(media_data_t(av_frame.data[0]
                        , av_frame.data[0] + av_frame.nb_samples * av_frame.channels * 2));

            break;
        }

        LOG_T << "Transcoder #" << context_id << ". Fetch PCM16 audio frame with size " << audio_data.size() << " bytes" LOG_END;

        av_packet.pts += av_frame.nb_samples;

        return audio_data;
    }

    bool set_audio_data(const void *data
                        , std::size_t size)
    {
        av_frame.nb_samples = size / audio_info_t::sample_size(av_frame.format, av_frame.channels);

        switch(av_frame.format)
        {
            case AV_SAMPLE_FMT_FLT:
            case AV_SAMPLE_FMT_FLTP:
            {
                resample_buffer.resize(size
                                       , av_frame.nb_samples * 2);

                utils::pcm16_to_float(data
                        , resample_buffer.data()
                        , av_frame.nb_samples);

                av_frame.data[0] = resample_buffer.data();
                av_frame.linesize[0] = resample_buffer.size();
            }
            break;
            case AV_SAMPLE_FMT_S16:
            case AV_SAMPLE_FMT_S16P:
                av_frame.data[0] = const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(data));
                av_frame.linesize[0] = size;
            break;
            default:
                av_frame.nb_samples = 0;
        }

        av_frame.pts += av_frame.nb_samples;

        LOG_T << "Transcoder #" << context_id << ". Put PCM16 audio frame with size " << av_frame.linesize[0] << " bytes" LOG_END;
        return av_frame.nb_samples > 0;
    }

    media_data_t get_video_data(std::int32_t align = default_frame_align)
    {
        std::size_t frame_size = av_image_get_buffer_size(static_cast<AVPixelFormat>(av_frame.format)
                                                          , av_frame.width
                                                          , av_frame.height
                                                          , align);

        media_data_t video_data(frame_size);

        if (frame_size > 0)
        {
            av_image_copy_to_buffer(video_data.data()
                                    , video_data.size()
                                    , av_frame.data
                                    , av_frame.linesize
                                    , static_cast<AVPixelFormat>(av_frame.format)
                                    , av_frame.width
                                    , av_frame.height
                                    , align
                                    );
        }

        LOG_T << "Transcoder #" << context_id << ". Fetch video frame with size " << video_data.size() << " bytes" LOG_END;

        av_packet.pts ++;

        return std::move(video_data);
    }

    bool set_video_data(const void* data
                                 , std::size_t size
                                 , std::int32_t align = default_frame_align)
    {
        std::size_t frame_size = av_image_fill_arrays(av_frame.data
                                                     , av_frame.linesize
                                                     , static_cast<const std::uint8_t*>(data)
                                                     , static_cast<AVPixelFormat>(av_frame.format)
                                                     , av_frame.width
                                                     , av_frame.height
                                                     , align);
        if (frame_size > size)
        {
            frame_size = 0;
            //LOG_T << "Transcoder #" << transcoder_id << ". Fetch video frame with size " << frame_size << " bytes" LOG_END;
        }


        LOG_T << "Transcoder #" << context_id << ". Put video frame with size " << frame_size << " bytes" LOG_END;

        av_frame.pts ++;

        return frame_size > 0;
    }

    media_data_t get_media_data()
    {
        media_data_t media_data;

        switch (av_context->codec_type)
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

    bool set_media_data(const void* data
                        , std::size_t size)
    {
        switch (av_context->codec_type)
        {
            case AVMEDIA_TYPE_AUDIO:
                return set_audio_data(data
                                      , size);
            break;
            case AVMEDIA_TYPE_VIDEO:
                return set_video_data(data
                                      , size);
            break;
            default:
                av_frame.data[0] = const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(data));
                av_frame.linesize[0] = size;
        }

        return true;
    }

    bool fill_frame_info(frame_t& frame
                         , bool is_encoder)
    {
        if (is_encoder
                ? av_packet.size > 0
                : av_frame.pkt_size > 0)
        {
            frame.info.pts = av_frame.pkt_pts;
            frame.info.dts = av_frame.pkt_dts;
            frame.info.id = frame_counter;

            if (av_context->codec->type == AVMEDIA_TYPE_AUDIO)
            {
                frame.info.media_info.media_type = media_type_t::audio;
                frame.info.media_info.audio_info.sample_rate = av_frame.sample_rate;
                frame.info.media_info.audio_info.channels = av_frame.channels;
                frame.info.media_info.audio_info.sample_format = av_frame.format;
            }
            else
            {
                frame.info.media_info.media_type = media_type_t::video;
                frame.info.media_info.video_info.size = { av_frame.width, av_frame.height };
                frame.info.media_info.video_info.fps = av_q2d(av_frame.sample_aspect_ratio) + 0.5;
                frame.info.media_info.video_info.pixel_format = av_frame.format;
            }

            if (is_encoder)
            {
                frame.info.codec_id = av_context->codec_id;
                frame.info.key_frame = (av_packet.flags & AV_PKT_FLAG_KEY) != 0;
                frame.media_data = media_data_t(av_packet.data
                                                , av_packet.data + av_packet.size);

            }
            else
            {
                frame.info.codec_id = codec_id_none;
                frame.info.key_frame = av_frame.key_frame;
                frame.media_data = get_media_data();

            }

            return !frame.media_data.empty();
        }

        return false;

    }

    bool decode(const void* data
                        , std::size_t size
                        , frame_queue_t& decoded_frames)
    {
        av_packet.data = const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(data));
        av_packet.size = size;

        auto result = avcodec_send_packet(av_context, &av_packet);

        if (result >= 0)
        {
            while (result >= 0)
            {
                result = avcodec_receive_frame(av_context, &av_frame);

                if (result >= 0)
                {
                    frame_t decoded_frame;

                    if (fill_frame_info(decoded_frame
                                        , false))
                    {
                        frame_counter++;
                        decoded_frames.push(std::move(decoded_frame));
                    }
                    else
                    {
                        LOG_W << "Transcoder #" << context_id << " decode null size frame" LOG_END;
                    }
                }
                else if (result != AVERROR(EAGAIN)
                         && result != AVERROR_EOF)
                {
                    LOG_E << "Transcoder #" << context_id << ". Error call avcodec_receive_frame, err = " << result LOG_END;
                }
                else
                {
                    return true;
                }
            }
        }
        else
        {
            LOG_E << "Transcoder #" << context_id << ". Error avcodec_send_packet, err = " << result LOG_END;
        }

        return false;
    }

    bool encode(const void* data
                , std::size_t size
                , frame_queue_t& encoded_frames
                , bool is_key_frame)
    {
        std::int32_t result = -1;

        if (set_media_data(data
                           , size))
        {
            av_frame.key_frame = static_cast<std::int32_t>(is_key_frame);

            result = avcodec_send_frame(av_context, &av_frame);

            if (result >= 0)
            {
                while (result >= 0)
                {
                    result = avcodec_receive_packet(av_context, &av_packet);

                    if (result >= 0)
                    {
                        frame_t encoded_frame;

                        if (fill_frame_info(encoded_frame
                                            , true))
                        {                            
                            frame_counter++;
                            encoded_frames.push(std::move(encoded_frame));
                        }
                        else
                        {
                            LOG_W << "Transcoder #" << context_id << " encode null size frame" LOG_END;
                        }
                    }
                    else if (result != AVERROR(EAGAIN)
                                               && result != AVERROR_EOF)
                    {
                        LOG_E << "Transcoder #" << context_id << ". Error call avcodec_receive_frame, err = " << result LOG_END;
                    }
                    else
                    {
                        return true;
                    }
                }

            }
            else
            {
                LOG_E << "Transcoder #" << context_id << ". Error avcodec_send_packet, err = " << result LOG_END;
            }

        }

        return false;
    }
};

struct libav_transcoder_context_t
{
    typedef std::unique_ptr<libav_codec_context_t> codec_context_ptr_t;

    codec_context_ptr_t         m_codec_context;
    stream_info_t               m_stream_info;
    transcoder_type_t           m_transcoder_type;

    libav_transcoder_context_t()
    {

    }

    bool open(const stream_info_t& steam_info
              , transcoder_type_t transcoder_type
              , const std::string& options)
    {
        close();
        if (transcoder_type != transcoder_type_t::unknown)
        {
            m_stream_info = steam_info;
            m_transcoder_type = transcoder_type;
            m_codec_context.reset(new libav_codec_context_t(m_stream_info
                                                            , m_transcoder_type == transcoder_type_t::encoder
                                                            , options));

            if (!m_codec_context->is_init)
            {
                close();
            }
        }

        return m_codec_context != nullptr;
    }

    bool close()
    {
        if (m_codec_context != nullptr)
        {
            m_codec_context.reset();
            m_transcoder_type = transcoder_type_t::unknown;
            m_stream_info = stream_info_t();

            return true;
        }

        return false;
    }

    bool is_open() const
    {
        return m_codec_context != nullptr;
    }

    bool transcode(const void* data
                   , std::size_t size
                   , frame_queue_t& frame_queue
                   , transcode_flag_t transcode_flags)
    {
        if (m_codec_context != nullptr)
        {
            switch(m_transcoder_type)
            {
                case transcoder_type_t::encoder:
                    return m_codec_context->encode(data
                                                   , size
                                                   , frame_queue
                                                   , transcode_flags & transcode_flag_t::key_frame);
                break;
                case transcoder_type_t::decoder:
                    return m_codec_context->decode(data
                                                   , size
                                                   , frame_queue);
                break;
            }
        }

        return false;
    }
};
//------------------------------------------------------------------------------
void libav_transcoder_context_deleter_t::operator()(libav_transcoder_context_t *libav_transcoder_context_ptr)
{
    delete libav_transcoder_context_ptr;
}
//------------------------------------------------------------------------------
libav_transcoder::libav_transcoder()
    : m_transcoder_context(new libav_transcoder_context_t())
{
    LOG_T << "Create libav transcoder" LOG_END;
}

bool libav_transcoder::open(const stream_info_t &steam_info
                            , transcoder_type_t transcoder_type
                            , const std::string& options)
{
    LOG_D << "Open transcoder by codec " << steam_info.codec_info.to_string() LOG_END;
    return m_transcoder_context->open(steam_info
                                      , transcoder_type
                                      , options);
}

bool libav_transcoder::close()
{
    LOG_D << "Close transcoder" LOG_END;
    return m_transcoder_context->close();
}

bool libav_transcoder::is_open() const
{
    return m_transcoder_context->is_open();
}

transcoder_type_t libav_transcoder::type() const
{
    return m_transcoder_context->m_transcoder_type;
}

const stream_info_t &libav_transcoder::config() const
{
    return m_transcoder_context->m_stream_info;
}

frame_queue_t libav_transcoder::transcode(const void *data
                                          , std::size_t size
                                          , transcode_flag_t transcode_flags)
{
    LOG_D << "Transcode frame size = " << size LOG_END;

    frame_queue_t frame_queue;

    m_transcoder_context->transcode(data
                                    , size
                                    , frame_queue
                                    , transcode_flags);

    return frame_queue;
}

bool libav_transcoder::transcode(const void *data
                                 , std::size_t size
                                 , frame_queue_t &frame_queue
                                 , transcode_flag_t transcode_flags)
{
    LOG_D << "Transcode frame size = " << size LOG_END;

    return m_transcoder_context->transcode(data
                                           , size
                                           , frame_queue
                                           , transcode_flags);
}

}
