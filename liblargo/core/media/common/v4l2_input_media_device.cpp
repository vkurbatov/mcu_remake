#include "v4l2_input_media_device.h"

#include "libav_input_media_device.h"
#include "media/video/video_frame.h"
#include "media/common/utils/format_converter.h"

#include <thread>
#include <algorithm>

namespace core
{

namespace media
{

auto fmt_to_string = [](const v4l2::frame_info_t& format)
{
    return video::video_format_t(utils::format_conversion::from_v4l2_format(format.pixel_format)
                                 , {format.size.width, format.size.height}
                                 , format.fps).to_string();
};

static void fetch_formats(control_parameter_list_t& controls
                          , const v4l2::format_list_t& v4l2_formats
                          , const v4l2::frame_info_t& current_format)
{
    variant_list_t list;

    for (const auto& fmt : v4l2_formats)
    {
        list.emplace_back(fmt_to_string(fmt));
    }

    controls.emplace_back(control_parameter("Resolution"
                                            , control_type_t::list
                                            , list
                                            , fmt_to_string(current_format)));
}

static void fetch_controls(control_parameter_list_t& controls
                           , const v4l2::control_list_t& v4l2_controls)
{
    std::uint32_t idx = 0;

    for (const auto& ctrl : v4l2_controls)
    {
        switch (ctrl.type())
        {
            case v4l2::control_type_t::numeric:
                controls.emplace_back(control_parameter(ctrl.name
                                                        , control_type_t::direct
                                                        , { ctrl.range.min, ctrl.range.max }
                                                        , ctrl.current_value
                                                        , idx));
            break;
            case v4l2::control_type_t::boolean:
                controls.emplace_back(control_parameter(ctrl.name
                                                    , control_type_t::check
                                                    , { }
                                                    , ctrl.current_value != 0
                                                    , idx));
            break;
            case v4l2::control_type_t::menu:
            {
                variant_list_t list;

                for(const auto& item : ctrl.menu)
                {
                    list.push_back(item.name);
                }

                controls.emplace_back(control_parameter(ctrl.name
                                                        , control_type_t::list
                                                        , list
                                                        , ctrl.current_value != 0
                                                        , idx));
            }
            break;
            default:
                continue;
        }

        idx ++;
    }
}

static media_frame_ptr_t create_video_frame(v4l2::frame_t&& frame)
{
    auto pixel_format = utils::format_conversion::from_v4l2_format(frame.frame_info.pixel_format);

    video::video_format_t video_format(pixel_format
                                       , { frame.frame_info.size.width, frame.frame_info.size.height }
                                       , frame.frame_info.fps);

    return video::video_frame::create(video_format
                                      , media_buffer::create(std::move(frame.frame_data)));
}

v4l2_input_media_device::v4l2_input_media_device(i_media_sink &media_sink
                                                 , std::uint32_t buffer_count)
    : m_buffer_count(buffer_count)
{
    auto frame_handler = [&media_sink](v4l2::frame_t&& v4l2_frame) ->
    bool
    {
        auto frame = create_video_frame(std::move(v4l2_frame));

        if (frame != nullptr)
        {
            media_sink.on_frame(*frame);
        }

        return true;
    };

    m_v4l2_device.reset(new v4l2::v4l2_device(frame_handler));
}

bool v4l2_input_media_device::open(const std::string &uri)
{
    if (m_v4l2_device->open(uri
                            , m_buffer_count))
    {

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        m_controls.clear();
        m_native_formats = m_v4l2_device->get_supported_formats();
        m_native_controls = m_v4l2_device->get_control_list();

        fetch_formats(m_controls
                      , m_native_formats
                      , m_v4l2_device->get_format());

        fetch_controls(m_controls
                       , m_native_controls);

        return true;
    }
    return false;
}

bool v4l2_input_media_device::close()
{
    return m_v4l2_device->close();
}

bool v4l2_input_media_device::is_open() const
{
    return m_v4l2_device->is_opened();
}

bool v4l2_input_media_device::is_established() const
{
    return m_v4l2_device->is_established();
}

const control_parameter_list_t& v4l2_input_media_device::controls() const
{
    return m_controls;
}

bool v4l2_input_media_device::set_control(const std::string &control_name
                                          , const variant control_value)
{
    auto it = std::find_if(m_controls.begin()
                           , m_controls.end()
                           , [&control_name](const control_parameter& param){ return param.name() == control_name; });

    if (it != m_controls.end())
    {
        control_parameter& ctrl = *it;

        if (control_name == "Resolution")
        {
            for (const auto& fmt : m_native_formats)
            {
                if (control_value == fmt_to_string(fmt))
                {
                    if (m_v4l2_device->set_format(fmt))
                    {
                        ctrl.set(control_value);
                        return true;
                    }
                }
            }
        }
        else if (control_name == "Port")
        {

        }
        else
        {
            if (ctrl.tag() < m_native_controls.size())
            {
                auto& native_ctrl = m_native_controls[ctrl.tag()];

                switch (native_ctrl.type())
                {
                    case v4l2::control_type_t::numeric:
                    case v4l2::control_type_t::boolean:
                        if (m_v4l2_device->set_control(native_ctrl.id
                                                       , control_value.get<std::int32_t>()))
                        {
                            ctrl.set(control_value);
                            return true;
                        }
                    break;
                    case v4l2::control_type_t::menu:
                    {
                        bool by_index = control_value.type() != variant_type_t::vt_string;

                        auto idx = 0;
                        for (const auto& item : native_ctrl.menu)
                        {
                            if (by_index
                                ? control_value == idx
                                : control_value == item.name)
                            {
                                if (m_v4l2_device->set_control(native_ctrl.id
                                                               , item.id))
                                {
                                    ctrl.set(control_value);
                                    return true;
                                }
                            }
                            idx++;
                        }
                    }
                    break;
                }
            }
        }
    }

    return false;
}

variant v4l2_input_media_device::get_control(const std::string &control_name
                                             , const variant default_value) const
{
    const auto it = std::find_if(m_controls.begin()
                           , m_controls.end()
                           , [&control_name](const control_parameter& param){ return param.name() == control_name; });

    if (it != m_controls.end())
    {
        const control_parameter& ctrl = *it;

        if (control_name == "Resolution")
        {
            return fmt_to_string(m_v4l2_device->get_format());
        }
        else
        {
            if (ctrl.tag() < m_native_controls.size())
            {
                const auto& native_ctrl = m_native_controls[ctrl.tag()];
                switch (native_ctrl.type())
                {
                    case v4l2::control_type_t::numeric:
                    case v4l2::control_type_t::boolean:
                        return m_v4l2_device->get_control(native_ctrl.id
                                                          , default_value.get<std::int32_t>());
                    break;
                    case v4l2::control_type_t::menu:
                    {
                        auto menu_id = m_v4l2_device->get_control(native_ctrl.id
                                                                  , native_ctrl.range.min - 1);

                        if (menu_id >= native_ctrl.range.min)
                        {
                            for (const auto& item : native_ctrl.menu)
                            {
                                if (item.id == menu_id)
                                {
                                    return item.name;
                                }
                            }
                        }

                    }
                    break;
                }

            }
        }
    }

    return default_value;
}

}

}
