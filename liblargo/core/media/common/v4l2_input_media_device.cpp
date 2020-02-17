#include "v4l2_input_media_device.h"

#include "libav_input_media_device.h"
#include "media/video/video_frame.h"
#include "media/common/utils/format_converter.h"
#include "media/common/serial/serial_device.h"

#include <thread>
#include <algorithm>

namespace core
{

namespace media
{


const std::int32_t visca_pan_min = -2448;
const std::int32_t visca_pan_max = 2448;
const std::int32_t visca_tilt_min = -432;
const std::int32_t visca_tilt_max = 1296;
const std::int32_t visca_zoom_min = 0;
const std::int32_t visca_zoom_max = 16384;

auto fmt_to_string = [](const v4l2::frame_info_t& format)
{
    return video::video_format_t(utils::format_conversion::from_v4l2_format(format.pixel_format)
                                 , {format.size.width, format.size.height}
                                 , format.fps).to_string();
};

static void fetch_formats(control_parameter_list_t& controls
                          , v4l2_device_ptr_t& v4l2_device)
{
    variant_list_t list;

    auto formats = v4l2_device->get_supported_formats();

    auto set_handler = [&v4l2_device, formats](variant& value) ->
    bool
    {
        auto idx = 0;
        bool by_index = value.type() != variant_type_t::vt_string;

        for (const auto& fmt : formats)
        {
            if (by_index
                    ? value == idx
                    : value == fmt_to_string(fmt))
            {
                return v4l2_device->set_format(fmt);
            }

            idx++;
        }
        return false;
    };

    auto get_handler = [&v4l2_device](variant& value) ->
    bool
    {
        value = fmt_to_string(v4l2_device->get_format());
        return true;
    };

    for (const auto& fmt : formats)
    {
        list.emplace_back(fmt_to_string(fmt));
    }

    controls.emplace_back(control_parameter("Resolution"
                                            , control_type_t::list
                                            , list
                                            , fmt_to_string(v4l2_device->get_format())
                                            , custom_parameter
                                            , set_handler
                                            , get_handler
                                            )
                          );
}

static void fetch_controls(control_parameter_list_t& controls
                           , v4l2_device_ptr_t& v4l2_device)
{
    auto v4l2_controls = v4l2_device->get_control_list();

    for (const auto& ctrl : v4l2_controls)
    {
        auto set_handler = [&v4l2_device, ctrl](variant& value) ->
        bool
        {
            switch(ctrl.type())
            {
                case v4l2::control_type_t::boolean:
                case v4l2::control_type_t::numeric:
                    return v4l2_device->set_control(ctrl.id
                                                    , value);
                break;
                case v4l2::control_type_t::menu:
                {

                    auto idx = 0;
                    bool by_index = value.type() != variant_type_t::vt_string;
                    for (const auto& item : ctrl.menu)
                    {
                        if (by_index
                            ? value == idx
                            : value == item.name)
                        {
                            return v4l2_device->set_control(ctrl.id
                                                            , item.id);
                        }
                    }
                }
                break;
            }
            return false;
        };

        auto get_handler = [&v4l2_device, ctrl](variant& value) ->
        bool
        {
            auto real_value = v4l2_device->get_control(ctrl.id
                                                  , ctrl.range.min - 1);

            if (real_value >= ctrl.range.min)
            {
                switch(ctrl.type())
                {
                    case v4l2::control_type_t::boolean:
                    {
                        value = real_value != 0;
                    }
                    break;
                    case v4l2::control_type_t::numeric:
                    {
                        value = real_value;
                    }
                    break;
                    case v4l2::control_type_t::menu:
                    {
                        value = ctrl.menu[real_value].name;
                    }
                    break;
                }

                return true;

            }
            return false;
        };

        variant_list_t list;
        variant current_value;
        control_type_t control_type;

        switch (ctrl.type())
        {
            case v4l2::control_type_t::boolean:
                list.push_back(false);
                list.push_back(true);
                current_value = ctrl.current_value;
                control_type = control_type_t::check;
            break;
            case v4l2::control_type_t::numeric:
                list.push_back(ctrl.range.min);
                list.push_back(ctrl.range.max);
                current_value = ctrl.current_value;
                control_type = control_type_t::direct;
            break;
            case v4l2::control_type_t::menu:
                for (const auto& item : ctrl.menu)
                {
                    list.push_back(item.name);
                    if (item.id == ctrl.default_value)
                    {
                        current_value = item.name;
                    }
                }
                control_type = control_type_t::list;
            break;
        }

        controls.emplace_back(control_parameter(ctrl.name
                                                , control_type
                                                , list
                                                , current_value
                                                , ctrl.id
                                                , set_handler
                                                , get_handler
                                                )
                              );
    }

}

static void fetch_visca_controls(control_parameter_list_t& controls
                                 , visca::visca_device& visca_device)
{
    variant_list_t port_list;

    for (auto const &s : serial::serial_device::serial_devices())
    {
        port_list.emplace_back(s);
    }

    controls.emplace_back(control_parameter("Visca port"
                                            , control_type_t::list
                                            , port_list
                                            , port_list.front()
                                            , custom_parameter
                                            , [&visca_device](variant& value) { visca_device.close(); return visca_device.open(value); }
                                            )
                          );

    controls.set("Visca port", port_list.front());

    controls.emplace_back(control_parameter("Visca pan absolute"
                                            , control_type_t::direct
                                            , { -2448, 2448 }
                                            , 0
                                            , custom_parameter
                                            , [&visca_device](variant& value) { return visca_device.set_pan(value); }
                                            , [&visca_device](variant& value) { std::int16_t pan = 0; if (!visca_device.get_pan(pan)) return false; value = pan; return true; }
                                            )
                          );

    controls.emplace_back(control_parameter("Visca tilt absolute"
                                            , control_type_t::direct
                                            , { -432, 1296 }
                                            , 0
                                            , custom_parameter
                                            , [&visca_device](variant& value) { return visca_device.set_tilt(value); }
                                            , [&visca_device](variant& value) { std::int16_t tilt = 0; if (!visca_device.get_tilt(tilt)) return false; value = tilt; return true; }
                                            )
                          );

    controls.emplace_back(control_parameter("Visca zoom absolute"
                                            , control_type_t::direct
                                            , { 0u, 16384u }
                                            , 0u
                                            , custom_parameter
                                            , [&visca_device](variant& value) { return visca_device.set_zoom(value); }
                                            , [&visca_device](variant& value) { std::uint16_t zoom = 0; if (!visca_device.get_zoom(zoom)) return false; value = zoom; return true; }
                                            )
                          );

}

struct camera_control_helper
{
    const std::int32_t uvc_control_factor = 250;

    enum class control_mode_t
    {
        uvc_a,
        uvc_b,
        visca
    };

    v4l2_device_ptr_t&          m_v4l2_device;
    visca::visca_device&        m_visca_device;
    control_mode_t              m_control_mode;


    std::int32_t            m_pan_min;
    std::int32_t            m_pan_max;
    std::int32_t            m_tilt_min;
    std::int32_t            m_tilt_max;
    std::int32_t            m_zoom_min;
    std::int32_t            m_zoom_max;

    camera_control_helper(v4l2_device_ptr_t& v4l2_device
                          , visca::visca_device& visca_device
                          , const std::string& control_mode)
        : m_v4l2_device(v4l2_device)
        , m_visca_device(visca_device)
        , m_control_mode(control_mode_t::visca)
        , m_pan_min(visca_pan_min)
        , m_pan_max(visca_pan_max)
        , m_tilt_min(visca_tilt_min)
        , m_tilt_max(visca_tilt_max)
        , m_zoom_min(visca_zoom_min)
        , m_zoom_max(visca_zoom_max)
    {
        if (control_mode == "UVC_A")
        {
            m_control_mode = control_mode_t::uvc_a;
        }
        else if (control_mode == "UVC_B")
        {
            m_control_mode = control_mode_t::uvc_b;
        }

        if (m_control_mode != control_mode_t::visca)
        {
            m_pan_min *= uvc_control_factor;
            m_pan_max *= uvc_control_factor;
            m_tilt_min *= uvc_control_factor;
            m_tilt_max *= uvc_control_factor;
        }
    }

    bool left()
    {
        switch (m_control_mode)
        {
            case control_mode_t::uvc_a: return m_v4l2_device->set_control(v4l2::ctrl_pan_absolute, m_pan_min); break;
            case control_mode_t::uvc_b: return m_v4l2_device->set_control(v4l2::ctrl_pan_speed, -10); break;
            case control_mode_t::visca: return m_visca_device.set_pan(m_pan_min); break;
        }

        return false;
    }

    bool right()
    {
        switch (m_control_mode)
        {
            case control_mode_t::uvc_a: return m_v4l2_device->set_control(v4l2::ctrl_pan_absolute, m_pan_max); break;
            case control_mode_t::uvc_b: return m_v4l2_device->set_control(v4l2::ctrl_pan_speed, 10); break;
            case control_mode_t::visca: return m_visca_device.set_pan(m_pan_max); break;
        }

        return false;
    }

    bool up()
    {
        switch (m_control_mode)
        {
            case control_mode_t::uvc_a: return m_v4l2_device->set_control(v4l2::ctrl_tilt_absolute, m_tilt_min); break;
            case control_mode_t::uvc_b: return m_v4l2_device->set_control(v4l2::ctrl_tilt_speed, -10); break;
            case control_mode_t::visca: return m_visca_device.set_tilt(m_tilt_min); break;
        }

        return false;
    }

    bool down()
    {
        switch (m_control_mode)
        {
            case control_mode_t::uvc_a: return m_v4l2_device->set_control(v4l2::ctrl_tilt_absolute, m_tilt_max); break;
            case control_mode_t::uvc_b: return m_v4l2_device->set_control(v4l2::ctrl_tilt_speed, 10); break;
            case control_mode_t::visca: return m_visca_device.set_tilt(m_tilt_max); break;
        }

        return false;
    }

    bool zoom_in()
    {
        switch (m_control_mode)
        {
            case control_mode_t::uvc_a: return m_v4l2_device->set_control(v4l2::ctrl_zoom_absolute, m_zoom_max); break;
            case control_mode_t::uvc_b: return m_v4l2_device->set_control(v4l2::ctrl_zoom_speed, 10); break;
            case control_mode_t::visca: return m_visca_device.set_zoom(m_zoom_max); break;
        }

        return false;
    }

    bool zoom_out()
    {
        switch (m_control_mode)
        {
            case control_mode_t::uvc_a: return m_v4l2_device->set_control(v4l2::ctrl_zoom_absolute, m_zoom_min); break;
            case control_mode_t::uvc_b: return m_v4l2_device->set_control(v4l2::ctrl_zoom_speed, -1); break;
            case control_mode_t::visca: return m_visca_device.set_zoom(m_zoom_min); break;
        }

        return false;
    }

    bool stop_move()
    {
        switch (m_control_mode)
        {
            case control_mode_t::uvc_a:
            case control_mode_t::uvc_b: return m_v4l2_device->set_control(v4l2::ctrl_pan_speed, 0) && m_v4l2_device->set_control(v4l2::ctrl_tilt_speed, 0) ; break;
            case control_mode_t::visca: return m_visca_device.pan_tilt_stop(); break;
        }

        return false;
    }

    bool stop_zoom()
    {
        switch (m_control_mode)
        {
            case control_mode_t::uvc_a:return m_v4l2_device->set_control(v4l2::ctrl_zoom_absolute, m_v4l2_device->get_control(v4l2::ctrl_zoom_absolute)); break;
            case control_mode_t::uvc_b: return m_v4l2_device->set_control(v4l2::ctrl_zoom_speed, 0); break;
            case control_mode_t::visca: return m_visca_device.zoom_stop(); break;
        }

        return false;
    }
};

static void fetch_custom_parameters(control_parameter_list_t& controls
                                    , v4l2_device_ptr_t& v4l2_device
                                    , visca::visca_device& visca_device)
{

    variant_list_t modes;

    if (controls.has_parameter("Pan (Absolute)"))
    {
        modes.push_back("UVC_A");
        modes.push_back("UVC_B");
    }

    modes.push_back("VISCA");

    controls.emplace_back(control_parameter("Control mode"
                                            , control_type_t::list
                                            , modes
                                            , modes.front()
                                            )
                          );

    enum class custom_control_type_t
    {
        pan,
        tilt,
        zoom,
        ptz
    };

    enum class control_mode_t
    {
        uvc_a,
        uvc_b,
        visca
    };

    auto set_handler = [&v4l2_device, &visca_device, &controls](variant& value
            , custom_control_type_t ctrl_type) -> bool
    {

        variant mode;
        controls.get("Control mode", mode);

        camera_control_helper control_helper(v4l2_device
                                             , visca_device
                                             , mode);

        switch (ctrl_type)
        {
            case custom_control_type_t::pan:
            {
                switch (value.get<std::int32_t>())
                {
                    case -1: return control_helper.left(); break;
                    case 0: return control_helper.stop_move(); break;
                    case 1: return control_helper.right(); break;
                }
            }
            case custom_control_type_t::tilt:
            {
                switch (value.get<std::int32_t>())
                {
                    case -1: return control_helper.up(); break;
                    case 0: return control_helper.stop_move(); break;
                    case 1: return control_helper.down(); break;
                }
            }
            case custom_control_type_t::zoom:
            {
                switch (value.get<std::int32_t>())
                {
                    case -1: return control_helper.zoom_out(); break;
                    case 0: return control_helper.stop_zoom(); break;
                    case 1: return control_helper.zoom_in(); break;
                }
            }
        };

        return false;
    };
/*
    auto get_handler = [&v4l2_device, &visca_device, &controls](variant& value
            , custom_control_type_t ctrl_type) -> bool
    {
        return false;
    };*/

    controls.emplace_back(control_parameter("Pan control"
                                            , control_type_t::direct
                                            , { -1, 1 }
                                            , 0
                                            , custom_parameter
                                            , std::bind(set_handler, std::placeholders::_1, custom_control_type_t::pan)
                                            )
                          );

    controls.emplace_back(control_parameter("Tilt control"
                                            , control_type_t::direct
                                            , { -1, 1 }
                                            , 0
                                            , custom_parameter
                                            , std::bind(set_handler, std::placeholders::_1, custom_control_type_t::tilt)
                                            )
                          );

    controls.emplace_back(control_parameter("Zoom control"
                                            , control_type_t::direct
                                            , { -1, 1 }
                                            , 0
                                            , custom_parameter
                                            , std::bind(set_handler, std::placeholders::_1, custom_control_type_t::zoom)
                                            )
                          );
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

        fetch_formats(m_controls
                      , m_v4l2_device);

        fetch_controls(m_controls
                       , m_v4l2_device);

        fetch_visca_controls(m_controls
                             , m_visca_device);

        fetch_custom_parameters(m_controls
                                , m_v4l2_device
                                , m_visca_device);

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

bool v4l2_input_media_device::set_control(const std::string& control_name
                                          , const variant control_value)
{
    return m_controls.set(control_name
                          , control_value);
}

variant v4l2_input_media_device::get_control(const std::string& control_name
                                             , const variant default_value) const
{
    variant result = default_value;

    m_controls.get(control_name
                   , result);

    return result;
}

}

}
