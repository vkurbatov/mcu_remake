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
struct libav_output_format_context_t
{
    struct AVFormatContext*     context;
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
            }
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
                    strm.codec_info.codec_params.set_global_header(true);
                }

                av_stream->id = strm.stream_id;
                av_stream->codecpar->bit_rate =  strm.codec_info.codec_params.bitrate;
                av_stream->codecpar->codec_id = static_cast<AVCodecID>(strm.codec_info.id);
                av_stream->codecpar->frame_size = strm.codec_info.codec_params.frame_size;

                av_stream->codecpar->codec_type = codec->type;

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

                        if ((strm.codec_info.codec_params.is_global_header())
                                && strm.extra_data.empty())
                        {
                            strm.extra_data = strm.extract_extra_data();
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
                    av_stream->codecpar->extradata_size = strm.extra_data.size() - AV_INPUT_BUFFER_PADDING_SIZE;
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
        // av_dump_format(context, 0, uri.c_str(), 1);

        if ((context->oformat->flags & AVFMT_NOFILE) == 0
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

            auto& av_stream = *context->streams[stream_id];

            static std::int64_t audio_pts = 0;
            static std::int64_t video_pts = 0;

            if (key_frame)
            {
                av_packet.flags |= AV_PKT_FLAG_KEY;
            }

            if (audio_pts == 0)
            {
                //audio_pts = AV_P;
                //video_pts = av_stream.pts;
            }

            av_packet.stream_index = stream_id;
            av_packet.data = const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(data));
            av_packet.size = size;

            if (av_stream.codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
            {
                // context->streams[stream_id]->pts.val += context->streams[stream_id]->codecpar->frame_size;
                av_packet.duration = 1024;
                av_packet.pts = audio_pts;
                av_packet_rescale_ts(&av_packet
                                     , { av_stream.pts.num, av_stream.pts.den }
                                     , av_stream.time_base);


                // av_packet.duration = (av_stream.codecpar->frame_size * 1000) / av_stream.time_base.den;
                /*av_packet_rescale_ts(&av_packet
                                     , av_stream.time_base
                                     , av_stream.time_base);*/

                audio_pts += av_stream.codecpar->frame_size;
            }
            else
            {
                // context->streams[stream_id]->pts.val ++;
                av_packet.duration = 0;
                av_packet.pts = video_pts;

                av_packet_rescale_ts(&av_packet
                                     , { av_stream.pts.num, av_stream.pts.den }
                                     , av_stream.time_base);
                video_pts++;
            }

            av_packet.dts = av_packet.pts;

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
