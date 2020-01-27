#include "libav_stream_publisher.h"

#include <thread>
#include <mutex>
#include <map>
#include <atomic>
#include <chrono>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
//#include <libavdevice/avdevice.h>
#include <libavutil/opt.h>
}

#define WBS_MODULE_NAME "ff:publisher"
#include <core-tools/logging.h>

namespace ffmpeg
{
/*
struct libav_stream_context
{
    struct AVStream* av_stream;

    libav_stream_context(AVFormatContext* format_context
                         , AVCodecID codec_id)
        : av_stream(avformat_new_stream(format_context
                                        , avcodec_find_encoder(codec_id)))
    {

    }

    ~libav_stream_context()
    {

    }
};
*/

struct libav_output_format_context_t
{
    struct AVFormatContext*     context;
    struct AVOutputFormat*      output_format;
    stream_info_list_t          streams;
    bool                        is_init;
    std::string                 uri;

    libav_output_format_context_t(const std::string& format_name
                           , const std::string& uri)
        : context(nullptr)
        , is_init(false)
        , uri(uri)
    {
        is_init = avformat_alloc_output_context2(&context
                                                 , nullptr
                                                 , format_name.c_str()
                                                 , uri.c_str()) >= 0;

        output_format = context->oformat;
    }

    ~libav_output_format_context_t()
    {
        if (context != nullptr)
        {
            avformat_free_context(context);
        }
    }

    bool add_stream(const stream_info_t& stream_info)
    {
        auto codec = avcodec_find_encoder(static_cast<AVCodecID>(stream_info.codec_info.id));

        if (codec != nullptr)
        {
            auto av_stream = avformat_new_stream(context
                                              , codec);

            if (av_stream != nullptr)
            {
                auto strm = stream_info;

                strm.stream_id = context->nb_streams - 1;

                if ((context->oformat->flags & AVFMT_GLOBALHEADER) != 0)
                {
                    strm.codec_info.codec_params.flags1 |= CODEC_FLAG_GLOBAL_HEADER;
                }

                av_stream->id = strm.stream_id;
                av_stream->codecpar->bit_rate =  strm.codec_info.codec_params.bitrate;
                av_stream->codecpar->codec_id = static_cast<AVCodecID>(strm.codec_info.id);
                av_stream->codecpar->frame_size = strm.codec_info.codec_params.frame_size;

                if (!strm.extra_data.empty())
                {
                    av_stream->codecpar->extradata = strm.extra_data.data();
                    av_stream->codecpar->extradata_size = strm.extra_data.size();
                }

                /*avcodec_parameters_from_context(av_stream->codecpar
                                                , av_stream->codec);*/

                av_stream->codecpar->codec_type = codec->type;

                switch (codec->type)
                {
                    case AVMEDIA_TYPE_AUDIO:
                        output_format->audio_codec = av_stream->codecpar->codec_id;
                        context->audio_codec = codec;
                        context->audio_codec_id = output_format->audio_codec;
                        av_stream->codecpar->sample_rate = stream_info.media_info.audio_info.sample_rate;
                        av_stream->codecpar->format = stream_info.media_info.audio_info.sample_format;
                        av_stream->codecpar->channels = stream_info.media_info.audio_info.channels;
                        av_stream->codecpar->channel_layout = av_stream->codecpar->channels > 1
                                                                ? AV_CH_LAYOUT_STEREO
                                                                : AV_CH_LAYOUT_MONO;

                        av_stream->time_base = { 1, av_stream->codecpar->sample_rate };/*av_d2q(stream_info.media_info.audio_info.sample_rate
                                                      , 48000);*/

                        av_stream->pts.val = 32000;


                    break;
                    case AVMEDIA_TYPE_VIDEO:
                        output_format->video_codec = av_stream->codecpar->codec_id;
                        context->video_codec = codec;
                        context->video_codec_id = output_format->audio_codec;

                        av_stream->codecpar->width  = stream_info.media_info.video_info.size.width;
                        av_stream->codecpar->height  = stream_info.media_info.video_info.size.height;
                        av_stream->codecpar->format = stream_info.media_info.video_info.pixel_format;
                        av_stream->codecpar->color_range = AVCOL_RANGE_MPEG;
                        av_stream->codecpar->sample_rate = 90000;
                        /*av_stream->codecpar->sample_aspect_ratio = av_d2q(stream_info.media_info.video_info.fps
                                                                          , 60);*/
                        av_stream->time_base = { 1, stream_info.media_info.video_info.fps };//av_stream->codecpar->sample_aspect_ratio;

                    break;
                    case AVMEDIA_TYPE_DATA:
                        output_format->data_codec = av_stream->codecpar->codec_id;
                    break;
                    case AVMEDIA_TYPE_SUBTITLE:
                        output_format->subtitle_codec = av_stream->codecpar->codec_id;
                    break;
                }

                av_stream->pts.den = av_stream->time_base.den;
                av_stream->pts.num = av_stream->time_base.num;

                streams.emplace_back(std::move(strm));
                return true;
            }

        }
        return false;
    }

    bool finish()
    {
        av_dump_format(context, 0, uri.c_str(), 1);

        if ((output_format->flags & AVFMT_NOFILE) == 0
                && avio_open(&context->pb, uri.c_str(), AVIO_FLAG_WRITE) < 0)
        {
            return false;
        }

        auto res = avformat_write_header(context
                                         , nullptr);

        if (res < 0)
        {
            return false;
        }

        return true;
    }

    bool push_frame(std::int32_t stream_id
                    , const void* data
                    , std::size_t size
                    , bool key_frame)
    {
        if (stream_id < context->nb_streams)
        {
            AVPacket av_packet = {};

            if (key_frame)
            {
                av_packet.flags |= AV_PKT_FLAG_KEY;
            }

            av_packet.stream_index = stream_id;
            av_packet.data = const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(data));
            av_packet.size = size;

            av_packet.pts = context->streams[stream_id]->pts.val;

            if (context->streams[stream_id]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
            {
                context->streams[stream_id]->pts.val += context->streams[stream_id]->codecpar->frame_size;
            }
            else
            {
                context->streams[stream_id]->pts.val ++;
            }

            // av_packet.pts = context->streams[stream_id]->codecpar->frame_size;

            LOG_D << "WRITE PACKET SIZE: " << av_packet.size LOG_END;

            return av_interleaved_write_frame(context, &av_packet) >= 0;
/*
            return av_write_frame(context
                                  , &av_packet) >= 0;*/
        }

        return false;
    }
};

struct libav_stream_publisher_context_t
{

    std::unique_ptr<libav_output_format_context_t> m_format_context;

    libav_stream_publisher_context_t()
    {

    }

    bool open(const std::string& uri
              , const stream_info_list_t& stream_list)
    {

        m_format_context.reset(new libav_output_format_context_t("flv"
                                                                 , uri));

        if (m_format_context->is_init)
        {
            for(const auto& strm: stream_list)
            {
                m_format_context->add_stream(strm);
            }

            if (!m_format_context->streams.empty())
            {
                if (m_format_context->finish())
                {
                    return true;
                }
            }
        }

        m_format_context.reset();
        return false;
    }

    bool close()
    {
        if (m_format_context != nullptr)
        {
            m_format_context.reset(nullptr);
            return true;
        }

        return false;
    }
    bool is_opened() const
    {
        return m_format_context != nullptr;
    }
    bool is_established() const
    {
        return is_opened();
    }

    stream_info_list_t streams() const
    {
        if (m_format_context != nullptr)
        {
            return m_format_context->streams;
        }

        return stream_info_list_t();
    }

    bool push_frame(std::int32_t stream_id
                    , const void* data
                    , std::size_t size
                    , bool key_frame)
    {
        return m_format_context->push_frame(stream_id
                                            , data
                                            , size
                                            , key_frame);
    }
};
//--------------------------------------------------------------------------
void libav_stream_publisher_context_deleter_t::operator()(libav_stream_publisher_context_t *libav_stream_publisher_context_ptr)
{
    delete libav_stream_publisher_context_ptr;
}
//--------------------------------------------------------------------------
libav_stream_publisher::libav_stream_publisher()
 : m_libav_stream_publisher_context(new libav_stream_publisher_context_t())
{

}

bool libav_stream_publisher::open(const std::string &uri
                                  , const stream_info_list_t &stream_list)
{
    return m_libav_stream_publisher_context->open(uri
                                                  , stream_list);
}

bool libav_stream_publisher::close()
{
    return m_libav_stream_publisher_context->close();
}

bool libav_stream_publisher::is_opened() const
{
    return m_libav_stream_publisher_context->is_opened();
}

bool libav_stream_publisher::is_established() const
{
    return m_libav_stream_publisher_context->is_established();
}

stream_info_list_t libav_stream_publisher::streams() const
{
    return m_libav_stream_publisher_context->streams();
}

bool libav_stream_publisher::push_frame(int32_t stream_id
                                        , const void *data
                                        , std::size_t size
                                        , bool key_frame)
{
    return m_libav_stream_publisher_context->push_frame(stream_id
                                                        , data
                                                        , size
                                                        , key_frame);
}

}
