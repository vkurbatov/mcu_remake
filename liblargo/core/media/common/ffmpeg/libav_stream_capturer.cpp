#include "libav_stream_capturer.h"
#include "libav_utils.h"

#include <thread>
#include <mutex>
#include <map>
#include <atomic>
#include <chrono>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavutil/opt.h>
}

#define WBS_MODULE_NAME "ff:capturer"
#include <core-tools/logging.h>

namespace ffmpeg
{

const std::size_t max_queue_size = 1000;
const std::size_t idle_timeout_ms = 10;

//------------------------------------------------------------------------------------

struct libav_input_format_context_t
{
struct AVFormatContext*     context;
struct AVPacket             packet;
std::uint32_t               context_id;
std::size_t                 total_read_bytes;
std::size_t                 total_read_frames;
device_type_t               type;

bool                        is_init;

libav_input_format_context_t()
    : context(nullptr)
    , context_id(0)
    , total_read_bytes(0)
    , total_read_frames(0)
    , type(device_type_t::unknown)
    , is_init(false)
{
    static std::uint32_t ctx_id = 0;
    context_id = ctx_id++;

    LOG_T << "Context #" << context_id << ". Create" LOG_END;

    av_init_packet(&packet);
}
~libav_input_format_context_t()
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

    auto c_uri = uri.c_str();

    auto device_type = utils::fetch_device_type(uri);

    if (!is_init)
    {
        AVDictionary* options = nullptr;

        type = device_type;
        switch(type)
        {
            case device_type_t::rtsp:
                av_dict_set_int(&options, "stimeout", 1000000, 0);
            break;
            case device_type_t::camera:
                av_dict_set(&options, "pixel_format", "mjpeg", 0);
                c_uri += 6;
                if (*c_uri != '/')
                {
                    c_uri++;
                }
            break;
        }

        result = avformat_open_input(&context
                                     , c_uri
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

                    stream_info.media_info.media_type = media_type_t::audio;

                    stream_info.media_info.audio_info.sample_rate = av_stream->codec->sample_rate;
                    stream_info.media_info.audio_info.channels = av_stream->codec->channels;
                    stream_info.media_info.audio_info.sample_format = static_cast<sample_format_t>(av_stream->codec->sample_fmt);

                break;
                case AVMEDIA_TYPE_VIDEO:

                    if (!is_video_allowed)
                    {
                        continue;
                    }

                    stream_info.media_info.media_type = media_type_t::video;

                    stream_info.media_info.video_info.size.width = av_stream->codec->width;
                    stream_info.media_info.video_info.size.height = av_stream->codec->height;

                    stream_info.media_info.video_info.fps = av_q2d(av_stream->avg_frame_rate) + 0.5;

                    if (stream_info.media_info.video_info.fps == 0)
                    {
                        stream_info.media_info.video_info.fps = av_q2d(av_stream->r_frame_rate) + 0.5;
                    }

                    stream_info.media_info.video_info.pixel_format = static_cast<pixel_format_t>(av_stream->codec->pix_fmt);
                    stream_info.codec_info.codec_params.gop = av_stream->codec->gop_size;
                break;

                case AVMEDIA_TYPE_DATA:
                case AVMEDIA_TYPE_SUBTITLE:

                    if (!is_data_allowed)
                    {
                        continue;
                    }
                    stream_info.media_info.media_type = media_type_t::data;
                break;
                default:
                    continue;
            }

            stream_info.stream_id = av_stream->index;
            if (av_stream->codec != nullptr)
            {
                stream_info.codec_info.id = av_stream->codec->codec_id;
                stream_info.codec_info.name = stream_info.codec_info.codec_name(stream_info.codec_info.id);
                stream_info.codec_info.codec_params.bitrate = av_stream->codec->bit_rate;
                stream_info.codec_info.codec_params.frame_size = av_stream->codec->frame_size;
                stream_info.codec_info.codec_params.flags1 = av_stream->codec->flags;
                stream_info.codec_info.codec_params.flags2 = av_stream->codec->flags2;
            }

            if (av_stream->codec->extradata != nullptr
                    && av_stream->codec->extradata_size > 0)
            {
                stream_info.extra_data = std::move(stream_info_t::create_extra_data(av_stream->codec->extradata
                                                                                    , av_stream->codec->extradata_size
                                                                                    , true));
            }

            streams.emplace_back(std::move(stream_info));
        }
    }

    return streams; 
}

std::int32_t fetch_media_data(frame_t& frame)
{
    std::int32_t result = -1;

    if (is_init)
    {
        result = av_read_frame(context, &packet);

        if (result >= 0 && packet.size > 0)
        {
            frame.info.dts = packet.dts;
            frame.info.dts = packet.pts;
            frame.info.id = packet.stream_index;
            frame.info.key_frame = (packet.flags & AV_PKT_FLAG_KEY) != 0;

            frame.media_data.resize(packet.size);

            memcpy(frame.media_data.data()
                        , packet.data
                        , packet.size);

            total_read_bytes += packet.size;
            total_read_frames++;

            result = packet.stream_index;
        }

        LOG_D << "Context #" << context_id << ". Fetch media data size " << packet.size
              << " from stream #" << result LOG_END;

        av_packet_unref(&packet);
    }
    else
    {
        LOG_W << "Context #" << context_id << ". Cant't fetch media data, context not init" LOG_END;
    }

    return result;
}

};
//------------------------------------------------------------------------------------
struct libav_stream_capturer_context_t
{
    struct libav_stream_t
    {
        stream_info_t   stream_info;
        frame_queue_t   frame_queue;
        std::mutex      queue_mutex;

        libav_stream_t(stream_info_t&& stream_info)
            : stream_info(std::move(stream_info))
        {

        }

        libav_stream_t(libav_stream_t&& other) = default;

        void push_data(frame_t&& frame)
        {
            std::lock_guard<std::mutex> lock(queue_mutex);

            while (frame_queue.size() >= max_queue_size)
            {
                frame_queue.pop();
            }

            frame_queue.emplace(std::move(frame));
        }

        frame_queue_t fetch_queue()
        {
            std::lock_guard<std::mutex> lock(queue_mutex);

            return std::move(frame_queue);
        }
    };

    std::string                                         m_uri;
    stream_data_handler_t                               m_stream_data_handler;
    stream_event_handler_t                              m_stream_event_handler;

    std::thread                                         m_stream_thread;
    std::map<std::int32_t,libav_stream_t>               m_streams;
    std::unique_ptr<libav_input_format_context_t>       m_format_context;

    std::mutex                                          m_mutex;
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
        std::lock_guard<std::mutex> lg(m_mutex);

        if (m_format_context == nullptr)
        {
            m_format_context.reset(new libav_input_format_context_t());

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
        std::lock_guard<std::mutex> lg(m_mutex);

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

        adaptive_timer_t delay_timer;
        // std::size_t frame_counter = 0;

        frame_t frame = {};

        while(m_is_running)
        {        
            if (is_open() || open(stream_mask))
            {
                auto result = m_format_context->fetch_media_data(frame);

                if (!m_is_running)
                {
                    break;
                }

                if (result >= 0)
                {
                    if (m_format_context->total_read_frames == 1)
                    {
                        delay_timer.reset();
                    }

                    error_counter = 0;

                    auto it = m_streams.find(result);

                    if (it != m_streams.end()
                            && !frame.media_data.empty())
                    {
                        const stream_info_t& stream_info = it->second.stream_info;
                        frame.info.media_info.media_type = stream_info.media_info.media_type;
                        frame.info.codec_id = stream_info.codec_info.id;

                        if (m_stream_data_handler == nullptr
                                || !m_stream_data_handler(stream_info
                                                         , std::move(frame.media_data))
                                )
                        {
                            LOG_D << "Capturer #" << m_capturer_id << ". Fetch data size " << frame.media_data.size()
                                  << " for " << stream_info.to_string() LOG_END;

                            it->second.push_data(std::move(frame));
                        }

                        if (m_format_context->type == device_type_t::file)
                        {
                            if (stream_info.media_info.media_type == media_type_t::video)
                            {
                                stream_info.media_info.video_info.fps != 0;
                                delay_timer.wait( 1000 / stream_info.media_info.video_info.fps);
                            }
                        }
                    }

                }
                else
                {
                    if (result == -EAGAIN || result == 0)
                    {
                        result = 0;
                        error_counter++;
                    }
                    else
                    {
                        LOG_W << "Capturer #" << m_capturer_id << ". Error fetch media data, err = " << result LOG_END;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(idle_timeout_ms));
                }

                if (error_counter > 10 || result < 0)
                {
                    LOG_E << "Capturer #" << m_capturer_id << ". Stopped, err = " << result LOG_END;
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

    frame_queue_t fetch_frame_queue(int32_t stream_id)
    {
        std::lock_guard<std::mutex> lg(m_mutex);

        frame_queue_t frame_queue;

        auto it = m_streams.find(stream_id);

        if (it != m_streams.end())
        {
            frame_queue = std::move(it->second.fetch_queue());
        }

        return std::move(frame_queue);
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

frame_queue_t libav_stream_capturer::fetch_media_queue(int32_t stream_id)
{
    frame_queue_t frame_queue;

    if (m_libav_stream_capturer_context != nullptr)
    {
        frame_queue = std::move(m_libav_stream_capturer_context->fetch_frame_queue(stream_id));
    }

    return std::move(frame_queue);
}
//------------------------------------------------------------------------------------

}
