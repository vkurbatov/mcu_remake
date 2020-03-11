#include "video_frame_processor.h"
#include "video_format.h"
#include "algorithm"


namespace core
{

namespace media
{

namespace video
{

video_frame_processor::video_frame_processor()
    : m_normalizer(media_format_t(video_info_t(pixel_format_t::yuv420p)))
{
    const static std::map<std::string, video::filters::flip_method_t> flip_methods =
    {
        { "none", video::filters::flip_method_t::none }
        , { "vertical", video::filters::flip_method_t::vertical }
        , { "horizontal", video::filters::flip_method_t::horizontal }
        , { "rotate", video::filters::flip_method_t::both }
    };

    auto set_flip_handler = [this](variant& value) -> bool
    {
        auto it = flip_methods.find(value);
        if (it != flip_methods.end())
        {
            m_flip_filter.set_flip_method(it->second);
            return true;
        }
        return false;
    };

    auto get_flip_handler = [this](variant& value) -> bool
    {
        auto it = std::find_if(flip_methods.begin(), flip_methods.end(), [this](const std::map<std::string, video::filters::flip_method_t>::value_type& value) { return m_flip_filter.flip_method() == value.second; } );
        if (it != flip_methods.end())
        {
            value = it->first;
            return true;
        }
        return false;
    };

    m_controls.emplace_back(control_parameter("Flip method"
                                               , control_type_t::list
                                               , { "none", "vertical", "horizontal", "rotate" }
                                               , "none"
                                               , custom_parameter
                                               , set_flip_handler
                                               , get_flip_handler
                                               ));
}

bool video_frame_processor::add_sink(media_sink_ptr_t sink_ptr)
{
    auto it = std::find(m_sinks.begin(), m_sinks.end(), sink_ptr);
    if (it == m_sinks.end())
    {
        m_sinks.push_back(sink_ptr);
        return true;
    }
    return false;
}

bool video_frame_processor::remove_sink(media_sink_ptr_t sink_ptr)
{
    auto it = std::find(m_sinks.begin(), m_sinks.end(), sink_ptr);
    if (it != m_sinks.end())
    {
        m_sinks.erase(it);
        return true;
    }
    return false;
}

const media_sink_list_t &video_frame_processor::sinks() const
{
    return m_sinks;
}

bool video_frame_processor::on_frame(media_frame_ptr_t frame)
{
    frame = m_normalizer.normalize(frame);

    if (frame != nullptr)
    {
        m_flip_filter.filter(*frame);

        for (auto& s : m_sinks)
        {
            if (s != nullptr)
            {
                s->on_frame(frame);
            }
        }

        return true;
    }

    return false;
}

const control_parameter_list_t &video_frame_processor::controls() const
{
    return m_controls;
}

bool video_frame_processor::set_control(const std::string &control_name
                                        , const variant &control_value)
{
    return m_controls.set(control_name
                          , control_value);
}

variant video_frame_processor::get_control(const std::string &control_name
                                           , const variant &default_value) const
{
    auto value = default_value;
    m_controls.get(control_name
                   , value);
    return value;
}

}

}

}
