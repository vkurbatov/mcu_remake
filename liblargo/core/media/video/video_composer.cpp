#include "video_composer.h"

#include "media/common/base/time_base.h"
#include "media/common/i_media_frame.h"
#include "media/video/video_frame.h"
#include "media/common/utils/format_converter.h"
#include "media/common/ffmpeg/libav_converter.h"
#include "media/video/filters/video_filter_overlay.h"
#include "media/video/filters/video_layer_figure.h"

#include <map>
#include <mutex>
#include <algorithm>

namespace core
{

namespace media
{

namespace video
{

const std::size_t max_streams = 16;
const std::uint64_t default_check_streams_interval = 1000;
const std::uint64_t default_stream_timeout = 2000;

struct video_composer_context_t
{
    struct stream_manager_t
    {
        struct stream_info_t
        {
            media_frame_ptr_t                               media_frame;
            base::adaptive_timer_t                          alive_timer;
            ffmpeg::libav_converter                         libav_converter;
            std::shared_ptr<filters::video_layer_figure>    figure_layer;
            filters::video_filter_overlay                   video_filter;
            double                                          weight;

            stream_info_t(media_frame_ptr_t frame = nullptr)
                : media_frame(frame)
                , libav_converter()
                , video_filter()
                , weight(0)
            {
                filters::figure_format_t figure_format(filters::figure_type_t::rectangle
                                                       , 0x00FF007F
                                                       , 0
                                                       , 16);

                figure_layer.reset(new filters::video_layer_figure(figure_format
                                                                   , { }));

                video_filter.set_overlays({ figure_layer });
            }

            bool check_alive(std::uint64_t timeout_ms)
            {
                return !alive_timer.wait(timeout_ms
                                         , false);
            }

            bool draw_frame(const relative_frame_rect_t& layout
                            , i_media_frame& output_frame
                            , bool is_selected = false)
            {

                auto align_framgent = [](ffmpeg::fragment_info_t& fragment
                        , std::int32_t align)
                {
                    fragment.frame_rect.offset.x -= fragment.frame_rect.offset.x % align;
                    fragment.frame_rect.size.width -= fragment.frame_rect.size.width % align;
                };

                if (media_frame != nullptr)
                {
                    auto& input_video_info = media_frame->media_format().video_info();
                    auto& output_video_info = output_frame.media_format().video_info();

                    ffmpeg::fragment_info_t input_framgent =
                    {
                        0
                        , 0
                        , input_video_info.size.width
                        , input_video_info.size.height
                        , input_video_info.size.width
                        , input_video_info.size.height
                        , utils::format_conversion::to_ffmpeg_video_format(input_video_info.pixel_format)
                    };

                    ffmpeg::fragment_info_t output_framgent =
                    {
                        layout.point.x * output_video_info.size.width
                        , layout.point.y * output_video_info.size.height
                        , layout.size.width * output_video_info.size.width
                        , layout.size.height * output_video_info.size.height
                        , output_video_info.size.width
                        , output_video_info.size.height
                        , utils::format_conversion::to_ffmpeg_video_format(output_video_info.pixel_format)
                    };


                    output_framgent.frame_rect.aspect_ratio(input_framgent.frame_rect);

                    align_framgent(input_framgent
                                   , 32);
                    align_framgent(output_framgent
                                   , 32);

                    libav_converter.convert_frames(input_framgent
                                                   , media_frame->data()
                                                   , output_framgent
                                                   , output_frame.data());

                    if (is_selected)
                    {
                        figure_layer->set_polylines({
                                                     { output_framgent.frame_rect.offset.x
                                                     , output_framgent.frame_rect.offset.y }
                                                     , { output_framgent.frame_rect.offset.x + output_framgent.frame_rect.size.width
                                                     , output_framgent.frame_rect.offset.y + output_framgent.frame_rect.size.height }
                                                     } );
                        video_filter.filter(output_frame);
                    }

                }
            }
        };

        typedef std::map<stream_id_t, stream_info_t> stream_info_map_t;

        stream_info_map_t                                       streams;
        std::vector<std::reference_wrapper<stream_info_t>>      active_streams;
        std::uint64_t                                           stream_timeout;
        base::adaptive_timer_t                                  adaptive_timer;

        stream_manager_t(std::uint64_t stream_timeout = default_stream_timeout)
            : stream_timeout(stream_timeout)
        {

        }

        void set_stream_weight(stream_id_t stream_id
                               , double weight)
        {
            auto it = streams.find(stream_id);
            if (it != streams.end())
            {
                it->second.weight = weight;
            }
        }

        bool push_frame(const i_media_frame &frame)
        {
            auto stream_id = frame.media_format().stream_id;
            auto it = streams.find(stream_id);

            bool result = it != streams.end();

            if (result)
            {
                if (it->second.media_frame->media_format().video_info()
                        != frame.media_format().video_info())
                {
                    active_streams.clear();
                }
                it->second.alive_timer.reset();
                it->second.media_frame = frame.clone();

            }
            else
            {
                if (streams.size() < max_streams)
                {
                    active_streams.clear();
                    streams.emplace(stream_id
                                    , stream_info_t(frame.clone()));
                    result = true;
                }
            }

            if (result)
            {
                check_and_update_streams();
            }

            return result;
        }

        bool check_and_update_streams()
        {
            bool result = false;

            if (adaptive_timer.wait(default_check_streams_interval
                                    , false))
            {

                auto it = streams.begin();

                while (it != streams.end())
                {
                    if (it->second.check_alive(stream_timeout))
                    {
                        it++;
                    }
                    else
                    {
                        it = streams.erase(it);
                    }
                }

                result = true;
            }

            result |= active_streams.empty()
                    && !streams.empty();

            if (result)
            {
                refresh_active_streams();
            }

            return result;
        }

        stream_id_t get_best_stream()
        {
            double weight = 0.0;
            stream_id_t stream_id = -1;

            for (auto& s : streams)
            {

            }

            return stream_id;
        }

        void refresh_active_streams()
        {
            active_streams.clear();
            for (auto& s : streams)
            {
                active_streams.emplace_back(std::ref(s.second));
            }
        }

        void reset()
        {
            active_streams.clear();
            streams.clear();
            adaptive_timer.reset();
        }

        video::video_info_t get_best_format() const
        {
            video::video_info_t video_format;
            for (const auto& s : streams)
            {
                const auto& s_video_format = s.second.media_frame->media_format().video_info();

                if (s_video_format.fps > video_format.fps)
                {
                    video_format.fps = s_video_format.fps;
                }

                if (s_video_format.size.size() > video_format.size.size())
                {
                    video_format.size = s_video_format.size;
                }
            }

            return video_format;
        }
    };

    media_format_t              m_output_format;
    media_frame_ptr_t           m_output_frame;

    i_video_layout_manager&     m_video_layout_manager;
    i_media_sink&               m_media_sink;
    bool                        m_is_enabled;
    frame_id_t                  m_frame_id;
    stream_manager_t            m_stream_manager;
    base::adaptive_timer_t      m_frame_timer;
    mutable std::mutex          m_mutex;

    video_composer_context_t(const media_format_t &output_format
                             , i_video_layout_manager& video_layout_manager
                             , i_media_sink& media_sink)
        : m_output_format(output_format)
        , m_video_layout_manager(video_layout_manager)
        , m_media_sink(media_sink)
        , m_is_enabled(false)
        , m_frame_id(0)
    {

    }

    bool activate(bool enable)
    {
        m_frame_id = 0;
        m_frame_timer.reset();
        m_is_enabled = enable;
        return true;
    }


    media_format_list_t get_streams() const
    {
        media_format_list_t streams;

        if (m_is_enabled)
        {
            std::lock_guard<std::mutex> lg(m_mutex);

            for (const auto& s : m_stream_manager.active_streams)
            {
                streams.push_back(s.get().media_frame->media_format());
            }
        }
        return streams;
    }

    void check_and_compose()
    {
        auto frame_time = 1000 / std::max(5u, m_output_format.video_info().fps);

        auto stream_count = m_stream_manager.active_streams.size();

        bool is_compose = m_frame_timer.wait(frame_time
                                             , false)
                || stream_count == 1;

        if (is_compose)
        {
            compose();
        }

    }

    void compose()
    {

        auto layout_id = m_stream_manager.active_streams.size();

        if (layout_id == 1)
        {
            m_media_sink.on_frame(*m_stream_manager.active_streams.front().get().media_frame);
            return;
        }

        if (layout_id > 1)
        {
            if (m_output_frame == nullptr
                     || m_output_frame->media_format().video_info() != m_output_format.video_info())
            {
                m_output_frame = video_frame::create(m_output_format
                                                     , nullptr
                                                     , 0);
            }

            m_output_frame->clear();

            stream_id_t stream_order = 0;

            for (auto& s : m_stream_manager.active_streams)
            {
                auto& stream = s.get();
                relative_frame_rect_t layout;

                if (m_video_layout_manager.fetch_layout(layout_id
                                                        , stream_order
                                                        , layout))
                {
                    stream.draw_frame(layout
                                      , *m_output_frame
                                      , stream.weight >= 0.5);

                }

                stream_order++;
            }

            m_output_frame->set_frame_id(m_frame_id++);
            m_media_sink.on_frame(*m_output_frame);
        }

    }

    bool push_frame(const i_media_frame &frame)
    {
        std::lock_guard<std::mutex> lg(m_mutex);

        if (frame.media_format().media_type == media_type_t::video
            && !frame.media_format().is_encoded())
        {
            auto result = m_stream_manager.push_frame(frame);

            check_and_compose();

            return result;
        }

        return false;
    }

    void set_stream_weight(stream_id_t stream_id
                           , double weight)
    {
        m_stream_manager.set_stream_weight(stream_id
                                           , weight);
    }
};

video_composer::video_composer(const media_format_t &output_format
                               , i_video_layout_manager& video_layout_manager
                               , i_media_sink& media_sink)
    : m_video_composer_context(new video_composer_context_t(output_format
                                                            , video_layout_manager
                                                            , media_sink
                                                            )
                               )
{

}

void video_composer::set_stream_weight(stream_id_t stream_id
                                       , double weight)
{
    return m_video_composer_context->set_stream_weight(stream_id
                                                       , weight);
}

bool video_composer::open(const std::string& /*uri*/)
{
    return m_video_composer_context->activate(true);
}

bool video_composer::close()
{
    return m_video_composer_context->activate(false);
}

bool video_composer::is_open() const
{
    return m_video_composer_context->m_is_enabled;
}

bool video_composer::is_established() const
{
    return m_video_composer_context->m_is_enabled;
}

media_format_list_t video_composer::streams() const
{
    return m_video_composer_context->get_streams();
}

const control_parameter_list_t &video_composer::controls() const
{

}

bool video_composer::set_control(const std::string &control_name
                                 , const variant &control_value)
{

}

variant video_composer::get_control(const std::string &control_name
                                    , const variant &default_value) const
{

}

bool video_composer::on_frame(const i_media_frame &frame)
{
    return m_video_composer_context->push_frame(frame);
}

}

}

}
