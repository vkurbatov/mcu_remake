#include "libav_stream_publisher.h"
#include "libav_utils.h"

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

static const char* fetch_stream_name(device_type_t device_type)
{
    static const char* format_table[] =
    {
        nullptr     // unknown,
        , "rtsp"    // rtsp,
        , "flv"     // rtmp,
        , "rtp"     // rtp,
        , "v4l2"    // camera,
        , nullptr     // http,
        , "mpeg"     // file,
    };

    return format_table[static_cast<std::int32_t>(device_type)];
}

struct libav_output_format_context_t
{
    struct AVFormatContext*     context;
    stream_info_list_t          streams;
    bool                        is_init;
    std::string                 uri;
    device_type_t               device_type;

    libav_output_format_context_t(const std::string& uri
                                  , const stream_info_list_t& stream_list)
        : context(nullptr)
        , is_init(false)
        , uri(uri)
        , device_type(utils::fetch_device_type(uri))
    {

        is_init = init(uri
                       , stream_list);

    }

    ~libav_output_format_context_t()
    {
        if (context != nullptr)
        {
            if (is_init)
            {
                av_write_trailer(context);
            }


            for (auto i = 0; i < context->nb_streams; i++)
            {
                context->streams[i]->codecpar->extradata = nullptr;
                context->streams[i]->codecpar->extradata_size = 0;

                if (context->streams[i]->codec != nullptr)
                {
                    avcodec_free_context(&context->streams[i]->codec);
                }
                av_freep(&context->streams[i]);
            }

            if ((context->oformat->flags & AVFMT_NOFILE) == 0)
            {
                avio_close(context->pb);
            }

            avformat_free_context(context);
        }
    }

    bool init(const std::string& uri
              , const stream_info_list_t& stream_list)
    {

        auto format = av_guess_format(nullptr
                                      , uri.c_str()
                                      , nullptr);

        if (device_type == device_type_t::file)
        {
            format = nullptr;
        }

        avformat_alloc_output_context2(&context
                                        , format
                                        , format == nullptr ? fetch_stream_name(device_type) : nullptr
                                        , uri.c_str());

        if (context != nullptr)
        {
            for(const auto& strm: stream_list)
            {
                add_stream(strm);
            }

            if (context->nb_streams > 0)
            {
                return finish_init();
            }
        }


        return false;
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
                    strm.codec_info.codec_params.set_global_header(true);
                }

                av_stream->id = strm.stream_id;
                av_stream->codecpar->bit_rate =  strm.codec_info.codec_params.bitrate;
                av_stream->codecpar->codec_id = static_cast<AVCodecID>(strm.codec_info.id);
                av_stream->codecpar->frame_size = strm.codec_info.codec_params.frame_size;

                av_stream->codecpar->codec_type = codec->type;

                av_stream->pts.val = 0;
                av_stream->time_base = { 1, video_sample_rate };

                switch (codec->type)
                {
                    case AVMEDIA_TYPE_AUDIO:
                        av_format_set_audio_codec(context
                                                  , codec);

                        context->oformat->audio_codec = av_stream->codecpar->codec_id;

                        stream_info.media_info >> *(av_stream->codecpar);

                        av_stream->time_base = { 1, av_stream->codecpar->sample_rate };

                    break;
                    case AVMEDIA_TYPE_VIDEO:

                        av_format_set_video_codec(context
                                                  , codec);

                        stream_info.media_info >> *(av_stream->codecpar);

                        av_stream->time_base = { 1, stream_info.media_info.video_info.fps };
                        av_stream->sample_aspect_ratio = av_stream->codecpar->sample_aspect_ratio;
                        av_stream->r_frame_rate = av_stream->codecpar->sample_aspect_ratio;

                        if ((strm.codec_info.codec_params.is_global_header())
                                && strm.extra_data.empty())
                        {
                            strm.extra_data = utils::extract_global_header(strm);
                        }

                    break;
                    case AVMEDIA_TYPE_DATA:
                        context->oformat->data_codec = av_stream->codecpar->codec_id;
                    break;
                    case AVMEDIA_TYPE_SUBTITLE:
                        context->oformat->subtitle_codec = av_stream->codecpar->codec_id;
                    break;
                }

                if (!strm.extra_data.empty())
                {
                    av_stream->codecpar->extradata = strm.extra_data.data();
                    av_stream->codecpar->extradata_size = strm.extra_data.size();// - AV_INPUT_BUFFER_PADDING_SIZE;
                }

                av_stream->pts.den = av_stream->time_base.den;
                av_stream->pts.num = av_stream->time_base.num;

                streams.emplace_back(std::move(strm));
                return true;
            }

        }
        return false;
    }

    bool finish_init()
    {
        av_dump_format(context, 0, uri.c_str(), 1);

        if ((context->oformat->flags & AVFMT_NOFILE) == 0)
        {
            if (avio_open(&context->pb, uri.c_str(), AVIO_FLAG_WRITE) < 0)
            {
                return false;
            }
        }

        auto res = avformat_write_header(context
                                         , nullptr);

        return res >= 0;
    }

    bool push_frame(std::int32_t stream_id
                    , const void* data
                    , std::size_t size
                    , bool key_frame)
    {
        if (stream_id < context->nb_streams)
        {
            AVPacket av_packet = {};

            auto& av_stream = *context->streams[stream_id];

            if (key_frame)
            {
                av_packet.flags |= AV_PKT_FLAG_KEY;
            }

            av_packet.stream_index = stream_id;
            av_packet.data = const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(data));
            av_packet.size = size;

            if (av_stream.codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
            {
                av_packet.duration = av_stream.codecpar->frame_size;
                av_packet.pts = av_packet.duration * av_stream.nb_frames;
                av_packet_rescale_ts(&av_packet
                                     , { av_stream.pts.num, av_stream.pts.den }
                                     , av_stream.time_base);



                // av_stream.pts.val += av_stream.codecpar->frame_size;
            }
            else
            {
                av_packet.pts = av_stream.nb_frames;

                av_packet_rescale_ts(&av_packet
                                     , { av_stream.pts.num, av_stream.pts.den }
                                     , av_stream.time_base);

                // av_stream.pts.val ++; //= av_stream.nb_frames;
            }

            av_packet.dts = AV_NOPTS_VALUE;//

            LOG_D << "WRITE STREAM [" << stream_id << "] PACKET SIZE: " << size << ". pts = " << av_packet.pts << ", flags = " << av_packet.flags LOG_END;

            auto ret = av_interleaved_write_frame(context, &av_packet);

            return ret >= 0;
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

        m_format_context.reset(new libav_output_format_context_t(uri
                                                                 , stream_list));

        if (m_format_context->is_init)
        {
            return true;
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
