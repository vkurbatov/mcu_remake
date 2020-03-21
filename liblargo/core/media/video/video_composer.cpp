#include "video_composer.h"

#include "media/common/base/time_base.h"
#include "media/common/i_media_frame.h"
#include "media/video/video_frame.h"
#include "media/common/utils/format_converter.h"
#include "media/common/ffmpeg/libav_converter.h"
#include "media/video/filters/video_filter_overlay.h"
#include "media/video/filters/video_layer_figure.h"
#include "media/video/video_layout_manager_custom.h"

#include <map>
#include <mutex>
#include <algorithm>
#include <functional>

namespace core
{

namespace media
{

namespace video
{

const std::size_t max_streams = 16;
const std::uint64_t default_check_streams_interval = 1000;
const std::uint64_t default_stream_timeout = 2000;

class selector_layout_manager : virtual public i_video_layout_manager
{


    // i_video_layout_manager interface
public:
    bool fetch_layout(layout_id_t layout_id
                      , stream_order_t order
                      , relative_frame_rect_t &layout) override
    {
        if (order == 0)
        {
            layout = { 0.0, 0.0, 1.0, 1.0 };
            return true;
        }

        return false;
    }
};

struct video_composer_context_t
{
    struct stream_manager_t
    {
        static ffmpeg::scaling_method_t smethod_form_qlevel(std::uint32_t qlevel)
        {
            switch(qlevel)
            {
                case 0:
                    return ffmpeg::scaling_method_t::point;
                break;
                case 1:
                    return ffmpeg::scaling_method_t::bilinear;
                break;
            }

            return ffmpeg::scaling_method_t::bicublin;
        }

        struct stream_info_t
        {
            const composer_config_t&                        config;
            media_frame_ptr_t                               media_frame;
            base::adaptive_timer_t                          alive_timer;
            ffmpeg::libav_converter                         libav_converter;
            std::shared_ptr<filters::video_layer_figure>    figure_layer;
            filters::video_filter_overlay                   video_filter;
            double                                          weight;

            stream_info_t(const composer_config_t& config
                          , media_frame_ptr_t frame = nullptr)
                : config(config)
                , media_frame(frame)
                , libav_converter(smethod_form_qlevel(config.scaling_quality))
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
                    auto dw = fragment.frame_rect.size.width % align;

                    fragment.frame_rect.size.width -= dw;
                    fragment.frame_rect.offset.x += dw / 2;

                    /*
                    auto dx = fragment.frame_rect.offset.x % align;

                    if (dx > 0)
                    fragment.frame_rect.offset.x -= dx;

                    auto dw = fragment.frame_rect.size.width % align;

                    if (dw > 0)
                    fragment.frame_rect.size.width += align - dw;*/

                    //fragment.frame_rect.size.height -= fragment.frame_rect.size.height % (align / 4);
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


                    if (config.clamp_aspect_ratio)
                    {
                        input_framgent.frame_rect.aspect_ratio(output_framgent.frame_rect);
                    }
                    else
                    {
                        output_framgent.frame_rect.aspect_ratio(input_framgent.frame_rect);
                    }

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
        base::adaptive_timer_t                                  adaptive_timer;
        const composer_config_t&                                config;

        stream_manager_t(const composer_config_t& config)
            : config(config)
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
                it->second.alive_timer.reset();
                it->second.media_frame = frame.clone();

            }
            else
            {
                if (streams.size() < max_streams)
                {
                    streams.emplace(stream_id
                                    , stream_info_t(config
                                                    , frame.clone()
                                                    )
                                    );
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
                    if (it->second.check_alive(config.stream_timeout))
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

            result |= !streams.empty();

            return result;
        }

        stream_id_t get_best_stream(stream_order_t& order) const
        {
            double weight = -1.0;
            stream_id_t stream_id = no_stream;

            stream_order_t i = 0;

            for (const auto& s : streams)
            {
                if (s.second.weight > weight)
                {
                    weight = s.second.weight;
                    stream_id = s.first;
                    order = i;
                }
                i++;
            }

            return stream_id;
        }


        void reset()
        {
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

    i_video_layout_manager&     m_custom_layout_manager;
    i_media_sink&               m_media_sink;
    composer_config_t           m_config;


    video_layout_manager_custom m_mosaic_layout_manager;
    bool                        m_is_enabled;
    frame_id_t                  m_frame_id;
    stream_manager_t            m_stream_manager;
    base::adaptive_timer_t      m_frame_timer;
    mutable std::mutex          m_mutex;

    video_composer_context_t(const media_format_t &output_format
                             , i_video_layout_manager& video_layout_manager
                             , i_media_sink& media_sink
                             , const composer_config_t& config)
        : m_output_format(output_format)
        , m_custom_layout_manager(video_layout_manager)
        , m_media_sink(media_sink)
        , m_config(config)
        , m_is_enabled(false)
        , m_frame_id(0)
        , m_stream_manager(m_config)
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

            for (const auto& s : m_stream_manager.streams)
            {
                streams.push_back(s.second.media_frame->media_format());
            }
        }
        return streams;
    }

    void check_and_compose()
    {
        auto frame_time = 1000 / std::max(5u, m_output_format.video_info().fps);

        auto stream_count = m_stream_manager.streams.size();

        bool is_compose = m_frame_timer.wait(frame_time
                                             , false)
                || stream_count == 1;

        if (is_compose)
        {
            compose();
        }

    }

    i_video_layout_manager& get_layout_manager()
    {
        static selector_layout_manager selecor_layout_manager;

        switch(m_config.layout_type)
        {
            case layout_type_t::mosaic:
                return m_mosaic_layout_manager;
            break;
            case layout_type_t::presenter:
                return m_custom_layout_manager;
            break;
            case layout_type_t::selector:
                return selecor_layout_manager;
            break;
        }

        return m_mosaic_layout_manager;
    }

    void compose()
    {

        auto layout_id = m_stream_manager.streams.size();

        if (layout_id == 1)
        {
            m_media_sink.on_frame(m_stream_manager.streams.begin()->second.media_frame);
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


            stream_order_t stream_counter = 0;
            stream_order_t kick_order = 0;
            stream_id_t best_stream = m_stream_manager.get_best_stream(kick_order);

            auto& layout_manager = get_layout_manager();

            for (auto& s : m_stream_manager.streams)
            {
                auto& stream = s.second;
                relative_frame_rect_t layout;

                bool is_select = m_config.vad_highlight
                        && stream.weight >= m_config.vad_level;

                auto stream_order = stream_counter++;

                if (m_config.layout_type != layout_type_t::mosaic)
                {
                    if (best_stream >= 0)
                    {
                        if (best_stream == s.first)
                        {
                            is_select = false;
                            stream_order = 0;
                        }
                        else if (stream_order == 0)
                        {
                            stream_order = kick_order;
                        }
                    }

                }

                if (layout_manager.fetch_layout(layout_id
                                                , stream_order
                                                , layout))
                {

                    stream.draw_frame(layout
                                      , *m_output_frame
                                      , is_select);
                }
            }

            m_output_frame->set_frame_id(m_frame_id++);
            m_media_sink.on_frame(m_output_frame);
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
                               , i_media_sink& media_sink
                               , const composer_config_t& config)
    : m_video_composer_context(new video_composer_context_t(output_format
                                                            , video_layout_manager
                                                            , media_sink
                                                            , config)
                               )
{
    static std::map<std::string, frame_size_t> resolutions =
    {
        { "800x600", { 800, 600 } }
        , { "1280x720", { 1280, 720 } }
        , { "1920x1080", { 1920, 1080 } }
        , { "3840×2160", { 3840, 2160 } }
    };

    static std::map<std::string, layout_type_t> layouts =
    {
        { "mosaic", layout_type_t::mosaic }
        , { "presenter", layout_type_t::presenter  }
        , { "selector", layout_type_t::selector  }
    };

    auto set_resolution = [this](variant& resolution) -> bool
    {
        auto it = resolutions.find(resolution);
        if (it != resolutions.end())
        {
            std::lock_guard<std::mutex> lg(m_video_composer_context->m_mutex);
            m_video_composer_context->m_output_format.video_info().size = it->second;
            return true;
        }

        return false;
    };

    auto get_resolution = [this](variant& resolution) -> bool
    {
        std::lock_guard<std::mutex> lg(m_video_composer_context->m_mutex);
        resolution = std::to_string(m_video_composer_context->m_output_format.video_info().size.width)
                + "x" + std::to_string(m_video_composer_context->m_output_format.video_info().size.height);
        return true;

    };

    auto set_layout_type = [this](variant& layout_type) -> bool
    {
        auto it = layouts.find(layout_type);
        if (it != layouts.end())
        {
            m_video_composer_context->m_config.layout_type = it->second;
            return true;
        }

        return false;
    };

    auto get_layout_type = [this](variant& layout_type) -> bool
    {
        auto it = std::find_if(layouts.begin(), layouts.end(), [this](const std::map<std::string, layout_type_t>::value_type& value) { return value.second == m_video_composer_context->m_config.layout_type; } );
        if (it != layouts.end())
        {
            layout_type = it->first;
            return true;
        }

        return false;
    };

    variant_list_t  resolution_list;

    for (const auto& r : resolutions)
    {
        resolution_list.emplace_back(r.first);
    }

    variant_list_t  layout_list;

    for (const auto& l : layouts)
    {
        layout_list.emplace_back(l.first);
    }

    m_controls.emplace_back(control_parameter("Resolution"
                                              , control_type_t::list
                                              , resolution_list
                                              , "1280x720"
                                              , custom_parameter
                                              , [set_resolution](variant& value){ return set_resolution(value); }
                                              , [get_resolution](variant& value){ return get_resolution(value); }
                                              ));

    m_controls.emplace_back(control_parameter("Layout"
                                              , control_type_t::list
                                              , layout_list
                                              , layout_list.front()
                                              , custom_parameter
                                              , [set_layout_type](variant& value){ return set_layout_type(value); }
                                              , [get_layout_type](variant& value){ return get_layout_type(value); }
                                              ));

    m_controls.emplace_back(control_parameter("Crop aspect ratio"
                                              , control_type_t::check
                                              , { false, true }
                                              , false
                                              , custom_parameter
                                              , [this](variant& value){ m_video_composer_context->m_config.clamp_aspect_ratio = value; return true; }
                                              , [this](variant& value){ value = m_video_composer_context->m_config.clamp_aspect_ratio; return true; }
                                              ));

   m_controls.emplace_back(control_parameter("Vad highlight"
                                              , control_type_t::check
                                              , { false, true }
                                              , false
                                              , custom_parameter
                                              , [this](variant& value){ m_video_composer_context->m_config.vad_highlight = value; return true; }
                                              , [this](variant& value){ value = m_video_composer_context->m_config.vad_highlight; return true; }
                                              ));

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
    return m_controls;
}

bool video_composer::set_control(const std::string &control_name
                                 , const variant &control_value)
{
    return m_controls.set(control_name
                          , control_value);
}

variant video_composer::get_control(const std::string &control_name
                                    , const variant &default_value) const
{
    auto value = default_value;
    return m_controls.get(control_name
                          , value);
}

bool video_composer::on_frame(media_frame_ptr_t frame)
{
    return m_video_composer_context->push_frame(*frame);
}

}

}

}
