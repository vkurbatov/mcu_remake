#include "libav_stream_capturer.h"

#include <thread>
#include <mutex>
#include <map>
#include <atomic>
#include <chrono>

extern "C"
{
#include <libavformat/avformat.h>
}

#define WBS_MODULE_NAME "ff:capturer"
#include <core-tools/logging.h>

namespace ffmpeg_wrapper
{

const std::size_t max_queue_size = 1000;
const std::size_t idle_timeout_ms = 10;
//------------------------------------------------------------------------------------
struct libav_format_context_t
{
struct AVFormatContext*     context;
struct AVPacket             packet;
std::uint32_t               context_id;
std::size_t                 total_read_bytes;
std::size_t                 total_read_frames;

bool                        is_init;

libav_format_context_t()
    : context(nullptr)
    , context_id(0)
    , total_read_bytes(0)
    , total_read_frames(0)
    , is_init(false)
{
    static std::uint32_t ctx_id = 0;
    context_id = ctx_id++;

    LOG_T << "Context #" << context_id << ". Create" LOG_END;

    av_init_packet(&packet);
}
~libav_format_context_t()
{
    LOG_T << "Context #" << context_id << ". Destroy" LOG_END;

    av_packet_unref(&packet);

    if (context != nullptr)
    {
        // av_read_pause(context);
        avformat_close_input(&context);
        avformat_free_context(context);

        LOG_I << "Context #" << context_id << ". Free resources" LOG_END;
    }
}

std::int32_t init(const std::string& uri)
{
    std::int32_t result = -1;

    bool is_rtsp = uri.find("rtsp:") == 0;

    if (!is_init)
    {
        AVDictionary* options = nullptr;
        if (is_rtsp)
        {
            av_dict_set_int(&options, "stimeout", 1000000, 0);

        }

        result = avformat_open_input(&context
                                     , uri.c_str()
                                     , nullptr
                                     , &options);

        if (result == 0)
        {
            result = avformat_find_stream_info(context
                                               , nullptr);

            is_init = result >= 0;

            if (is_init)
            {
                LOG_I << "Context #" << context_id << ". Open streams (" << context->nb_streams << ") success" LOG_END;
            }
            else
            {
                LOG_E << "Context #" << context_id << ". Open streams failed, err = " << result LOG_END;
            }
        }
    }

    return result;
}

stream_info_list_t get_streams(stream_mask_t stream_mask)
{
    stream_info_list_t streams;

    bool is_audio_allowed = (stream_mask & stream_mask_t::stream_mask_audio) != stream_mask_empty;
    bool is_video_allowed = (stream_mask & stream_mask_t::stream_mask_video) != stream_mask_empty;
    bool is_data_allowed = (stream_mask & stream_mask_t::stream_mask_data) != stream_mask_empty;

    if (is_init)
    {
        for (unsigned i = 0; i < context->nb_streams; i++)
        {
            auto av_stream = context->streams[i];

            stream_info_t stream_info = {};

            switch(av_stream->codec->codec_type)
            {
                case AVMEDIA_TYPE_AUDIO:

                    if (!is_audio_allowed)
                    {
                        continue;
                    }

                    stream_info.media_type = media_type_t::audio;

                    stream_info.media_info.audio_info.sample_rate = av_stream->codec->sample_rate;
                    stream_info.media_info.audio_info.channels = av_stream->codec->channels;
                    stream_info.media_info.audio_info.sample_format = static_cast<sample_format_t>(av_stream->codec->sample_fmt);

                break;
                case AVMEDIA_TYPE_VIDEO:

                    if (!is_video_allowed)
                    {
                        continue;
                    }

                    stream_info.media_type = media_type_t::video;

                    stream_info.media_info.video_info.size.width = av_stream->codec->width;
                    stream_info.media_info.video_info.size.height = av_stream->codec->height;
                    stream_info.media_info.video_info.fps = av_stream->codec_info_nb_frames;
                    stream_info.media_info.video_info.pixel_format = static_cast<pixel_format_t>(av_stream->codec->pix_fmt);
                break;

                case AVMEDIA_TYPE_DATA:
                case AVMEDIA_TYPE_SUBTITLE:

                    if (!is_data_allowed)
                    {
                        continue;
                    }
                    stream_info.media_type = media_type_t::data;
                break;
                default:
                    continue;
            }

            stream_info.stream_id = av_stream->index;
            stream_info.codec_info.id = av_stream->codec->codec_id;
            stream_info.codec_info.name = avcodec_get_name(av_stream->codec->codec_id);

            if (av_stream->codec->extradata != nullptr
                    && av_stream->codec->extradata_size > 0)
            {
                stream_info.codec_info.extra_data = std::move(media_data_t(av_stream->codec->extradata
                                                                           , av_stream->codec->extradata + av_stream->codec->extradata_size));
            }

            streams.emplace_back(std::move(stream_info));
        }
    }

    return streams; 
}

std::pair<std::int32_t, media_data_t> fetch_media_data()
{
    std::pair<std::int32_t, media_data_t> media_data = { -1, media_data_t() };

    if (is_init)
    {
        packet.size = 0;

        media_data.first = av_read_frame(context, &packet);

        if (media_data.first >= 0 && packet.size > 0)
        {
            media_data.first = packet.stream_index;
            media_data.second = std::move(media_data_t(packet.data
                                                       , packet.data + packet.size));
            total_read_bytes += packet.size;
            total_read_frames++;
        }

        LOG_D << "Context #" << context_id << ". Fetch media data size " << packet.size
              << " from stream #" << media_data.first LOG_END;

        av_packet_unref(&packet);
    }
    else
    {
        LOG_W << "Context #" << context_id << ". Cant't fetch media data, context not init" LOG_END;
    }

    return std::move(media_data);
}

};
//------------------------------------------------------------------------------------
struct libav_stream_capturer_context_t
{
    struct libav_stream_t
    {
        stream_info_t   stream_info;
        media_queue_t   media_queue;
        std::mutex      queue_mutex;

        libav_stream_t(stream_info_t&& stream_info)
            : stream_info(std::move(stream_info))
        {

        }

        libav_stream_t(libav_stream_t&& other) = default;

        void push_data(media_data_t&& media_data)
        {
            std::lock_guard<std::mutex> lock(queue_mutex);

            while (media_queue.size() >= max_queue_size)
            {
                media_queue.pop();
            }

            media_queue.emplace(std::move(media_data));
        }

        media_queue_t fetch_queue()
        {
            std::lock_guard<std::mutex> lock(queue_mutex);

            return std::move(media_queue);
        }
    };

    std::string                                         m_uri;
    stream_data_handler_t                               m_stream_data_handler;
    stream_event_handler_t                              m_stream_event_handler;

    std::thread                                         m_stream_thread;
    std::map<std::int32_t,libav_stream_t>               m_streams;
    std::unique_ptr<libav_format_context_t>             m_format_context;

    std::mutex                                          m_queue_mutex; 
    std::atomic_bool                                    m_is_running;

    std::uint32_t                                       m_capturer_id;

    libav_stream_capturer_context_t(const std::string& uri
                                    , stream_data_handler_t stream_data_handler
                                    , stream_event_handler_t stream_event_handler
                                    , stream_mask_t stream_mask)
        : m_uri(uri)
        , m_stream_data_handler(stream_data_handler)
        , m_stream_event_handler(stream_event_handler)
        , m_format_context(nullptr)
        , m_is_running(false)
    {
        std::uint32_t cap_id = 0;
        m_capturer_id = cap_id++;

        LOG_T << "Capturer #" << m_capturer_id << ". Create for uri " << uri LOG_END;

        m_is_running = true;
        m_stream_thread = std::thread(&libav_stream_capturer_context_t::streamig_proc, this, stream_mask);
    }

    ~libav_stream_capturer_context_t()
    {
        LOG_T << "Capturer #" << m_capturer_id << ". Destroy" LOG_END;
        stop();
    }

    bool open(stream_mask_t stream_mask)
    {
        if (m_format_context == nullptr)
        {
            m_format_context.reset(new libav_format_context_t());

            if (m_format_context->init(m_uri) >= 0)
            {
                m_streams.clear();

                for (auto& s_info : m_format_context->get_streams(stream_mask))
                {

                    LOG_D << "Capturer #" << m_capturer_id << ". Add stream "
                          << s_info.to_string() LOG_END;


                    m_streams.emplace(std::piecewise_construct
                                      , std::forward_as_tuple(s_info.stream_id)
                                      , std::forward_as_tuple(std::move(s_info)));
                }

                push_event(streaming_event_t::open);
            }
            else
            {
                m_format_context.reset(nullptr);
                LOG_E << "Capturer #" << m_capturer_id << ". Can't initialize context" LOG_END;
            }
        }

        return is_open();
    }

    bool is_open() const
    {
        return m_format_context != nullptr && !m_streams.empty();
    }

    bool close()
    {
        if (m_format_context != nullptr)
        {
            m_format_context.reset(nullptr);
            m_streams.clear();
            push_event(streaming_event_t::close);
        }
    }

    void streamig_proc(stream_mask_t stream_mask)
    {

        LOG_I << "Capturer #" << m_capturer_id << ". Started" LOG_END;

        push_event(streaming_event_t::start);

        std::uint32_t error_counter = 0;        

        while(m_is_running)
        {        
            if (is_open() || open(stream_mask))
            {
                auto media_data = std::move(m_format_context->fetch_media_data());

                if (!m_is_running)
                {
                    break;
                }

                if (!media_data.second.empty())
                {
                    error_counter = 0;

                    auto it = m_streams.find(media_data.first);

                    if (it != m_streams.end()
                            && !media_data.second.empty())
                    {
                        if (m_stream_data_handler == nullptr
                                || !m_stream_data_handler(it->second.stream_info
                                                         , std::move(media_data.second))
                                )
                        {
                            LOG_D << "Capturer #" << m_capturer_id << ". Fetch data size " << media_data.second.size()
                                  << " for " << it->second.stream_info.to_string() LOG_END;

                            it->second.push_data(std::move(media_data.second));
                        }
                    }
                }
                else
                {
                    if (media_data.first == -EAGAIN || media_data.first == 0)
                    {
                        media_data.first = 0;
                        error_counter++;
                    }
                    else
                    {
                        LOG_W << "Capturer #" << m_capturer_id << ". Error fetch media data, err = " << media_data.first LOG_END;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(idle_timeout_ms));
                }

                if (error_counter > 10 || media_data.first < 0)
                {
                    LOG_E << "Capturer #" << m_capturer_id << ". Stopped, err = " << media_data.first LOG_END;
                    close();
                }

            }
        }

        close();

        LOG_I << "Capturer #" << m_capturer_id << ". Stopped" LOG_END;
        push_event(streaming_event_t::stop);
    }

    void stop()
    {
        if (m_is_running)
        {
            LOG_I << "Capturer #" << m_capturer_id << ". Stopping..." LOG_END;

            m_is_running = false;

            if (m_stream_thread.joinable())
            {
                m_stream_thread.join();
            }
        }
    }

    media_queue_t fetch_media_queue(int32_t stream_id)
    {
        media_queue_t media_queue;

        auto it = m_streams.find(stream_id);

        if (it != m_streams.end())
        {
            media_queue = std::move(it->second.fetch_queue());
        }

        return std::move(media_queue);
    }

    void push_event(streaming_event_t capture_event)
    {
        if (m_stream_event_handler != nullptr)
        {
            m_stream_event_handler(capture_event);
        }
    }
};
//------------------------------------------------------------------------------------
void libav_stream_capturer_context_deleter_t::operator()(libav_stream_capturer_context_t *libav_stream_capturer_context_ptr)
{
    delete libav_stream_capturer_context_ptr;
}
//------------------------------------------------------------------------------------
libav_stream_capturer::libav_stream_capturer(stream_data_handler_t stream_data_handler
                                             , stream_event_handler_t stream_event_handler)
    : m_stream_data_handler(stream_data_handler)
    , m_stream_event_handler(stream_event_handler)
{

}

bool libav_stream_capturer::open(const std::string &uri
                                 , stream_mask_t stream_mask)
{
    m_libav_stream_capturer_context.reset(new libav_stream_capturer_context_t(uri
                                              , m_stream_data_handler
                                              , m_stream_event_handler
                                              , stream_mask)
                          );

    return true;
}

bool libav_stream_capturer::close()
{
    m_libav_stream_capturer_context.reset();

    return true;
}

bool libav_stream_capturer::is_opened() const
{
    return m_libav_stream_capturer_context != nullptr;
}

bool libav_stream_capturer::is_established() const
{
    return m_libav_stream_capturer_context != nullptr
            && m_libav_stream_capturer_context->is_open();
}

stream_info_list_t libav_stream_capturer::streams() const
{
    stream_info_list_t stream_info_list;

    if (m_libav_stream_capturer_context != nullptr)
    {
        for (const auto& stream : m_libav_stream_capturer_context->m_streams)
        {
            stream_info_list.emplace_back(stream.second.stream_info);
        }
    }

    return std::move(stream_info_list);
}

media_queue_t libav_stream_capturer::fetch_media_queue(int32_t stream_id)
{
    media_queue_t media_queue;

    if (m_libav_stream_capturer_context != nullptr)
    {
        media_queue = std::move(m_libav_stream_capturer_context->fetch_media_queue(stream_id));
    }

    return std::move(media_queue);
}
//------------------------------------------------------------------------------------

}
