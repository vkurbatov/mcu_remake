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

class i_camera_control
{
public:
    virtual ~i_camera_control(){}
    virtual bool control_pan(std::int32_t direction) = 0;
    virtual bool control_tilt(std::int32_t direction) = 0;
    virtual bool control_zoom(std::int32_t direction) = 0;
    virtual bool set_preset(std::uint64_t preset) = 0;
    virtual bool get_preset(std::uint64_t& preset) = 0;
};

class camera_control : virtual public i_camera_control
{
public:
    virtual bool set_preset(std::uint64_t preset)
    {
        double pan = static_cast<double>((preset >> 40) & 0xffffff) / static_cast<double>(0xffffff);
        double tilt = static_cast<double>((preset >> 16) & 0xffffff) / static_cast<double>(0xffffff);
        double zoom = static_cast<double>((preset >> 0) & 0xffff) / static_cast<double>(0xffff);

        return set_ptz_state(pan, tilt, zoom);
    }
    virtual bool get_preset(std::uint64_t& preset)
    {
        double pan = 0, tilt = 0, zoom = 0;

        if (get_ptz_state(pan, tilt, zoom))
        {
            preset = 0;
            preset |= static_cast<std::uint64_t>(pan * 0xffffff) << 40;
            preset |= static_cast<std::uint64_t>(tilt * 0xffffff) << 16;
            preset |= static_cast<std::uint64_t>(zoom * 0xffff) << 0;

            return true;
        }

        return false;
    }
protected:
    virtual bool get_ptz_state(double& pan
                               , double& tilt
                               , double& zoom) = 0;

    virtual bool set_ptz_state(double pan
                               , double tilt
                               , double zoom) = 0;
};

class uvc_camera_control : public camera_control
{
    v4l2::v4l2_device&      m_v4l2_device;
    bool                    m_mode_b;

public:
    uvc_camera_control(v4l2::v4l2_device& v4l2_device
                       , bool mode_b = false)
        : m_v4l2_device(v4l2_device)
        , m_mode_b(mode_b)
    {

    }
    // i_camera_control interface
public:
    bool control_pan(int32_t direction) override
    {
        if (m_v4l2_device.is_opened())
        {
            switch (direction)
            {
                case -1:
                    return m_mode_b
                            ? m_v4l2_device.set_control(v4l2::ctrl_pan_speed, -8)
                            : m_v4l2_device.set_relative_control(v4l2::ctrl_pan_absolute, 0);
                break;
                case 0:
                    return m_v4l2_device.set_control(v4l2::ctrl_pan_speed, 0);
                break;
                case 1:
                return m_mode_b
                        ? m_v4l2_device.set_control(v4l2::ctrl_pan_speed, 8)
                        : m_v4l2_device.set_relative_control(v4l2::ctrl_pan_absolute, 1);
                break;
            }
        }

        return false;
    }
    bool control_tilt(int32_t direction) override
    {
        switch (direction)
        {
            case -1:
                return m_mode_b
                        ? m_v4l2_device.set_control(v4l2::ctrl_tilt_speed, -10)
                        : m_v4l2_device.set_relative_control(v4l2::ctrl_tilt_absolute, 0);
            break;
            case 0:
                return m_v4l2_device.set_control(v4l2::ctrl_tilt_speed, 0);
            break;
            case 1:
                return m_v4l2_device.set_relative_control(v4l2::ctrl_tilt_absolute, 1);
            break;
        }

        return false;

    }
    bool control_zoom(int32_t direction) override
    {
        switch (direction)
        {
            case -1:
                return m_mode_b
                        ? m_v4l2_device.set_control(v4l2::ctrl_zoom_speed, -10)
                        : m_v4l2_device.set_relative_control(v4l2::ctrl_zoom_absolute, 0);
            break;
            case 0:
                return m_v4l2_device.set_control(v4l2::ctrl_zoom_speed, 0)
                        && m_mode_b || m_v4l2_device.set_control(v4l2::ctrl_zoom_absolute, m_v4l2_device.get_control(v4l2::ctrl_zoom_absolute));
            break;
            case 1:
                return m_v4l2_device.set_control(v4l2::ctrl_zoom_speed, 10);
            break;
        }

        return false;
    }
    // camera_control interface
protected:
    bool get_ptz_state(double &pan, double &tilt, double &zoom) override
    {
        return m_v4l2_device.get_ptz(pan, tilt, zoom);
    }
    bool set_ptz_state(double pan, double tilt, double zoom) override
    {
        return m_v4l2_device.set_ptz(pan, tilt, zoom);
    }
};

class visca_camera_control : public camera_control
{
    visca::visca_device&    m_visca_device;

public:
    visca_camera_control(visca::visca_device& visca_device)
        : m_visca_device(visca_device)
    {

    }
    // i_camera_control interface
public:
    bool control_pan(int32_t direction) override
    {
        if (m_visca_device.is_opened())
        {
            switch (direction)
            {
                case -1:
                    return m_visca_device.set_pan(visca::visca_pan_min);
                break;
                case 0:
                    return m_visca_device.pan_tilt_stop();
                break;
                case 1:
                    return m_visca_device.set_pan(visca::visca_pan_max);
                break;
            }
        }

        return false;
    }

    bool control_tilt(int32_t direction) override
    {
        if (m_visca_device.is_opened())
        {
            switch (direction)
            {
                case -1:
                    return m_visca_device.set_tilt(visca::visca_tilt_min);
                break;
                case 0:
                    return m_visca_device.pan_tilt_stop();
                break;
                case 1:
                    return m_visca_device.set_tilt(visca::visca_tilt_max);
                break;
            }
        }

        return false;
    }

    bool control_zoom(int32_t direction) override
    {
        if (m_visca_device.is_opened())
        {
            switch (direction)
            {
                case -1:
                    return m_visca_device.set_zoom(visca::visca_zoom_min);
                break;
                case 0:
                    return m_visca_device.zoom_stop();
                break;
                case 1:
                    return m_visca_device.set_zoom(visca::visca_zoom_max);
                break;
            }
        }

        return false;
    }

    // camera_control interface
protected:
    bool get_ptz_state(double &pan, double &tilt, double &zoom) override
    {
        return m_visca_device.get_ptz(pan, tilt, zoom);
    }
    bool set_ptz_state(double pan, double tilt, double zoom) override
    {
        return m_visca_device.set_ptz(pan, tilt, zoom);
    }
};

auto fmt_to_string = [](const v4l2::frame_info_t& format)
{
    return media_format_t(video::video_info_t(utils::format_conversion::from_v4l2_video_format(format.pixel_format)
                                                , {format.size.width, format.size.height}
                                                , format.fps)).to_string();
};

static void fetch_formats(control_parameter_list_t& controls
                          , v4l2::v4l2_device& v4l2_device)
{
    variant_list_t list;

    auto formats = v4l2_device.get_supported_formats();

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
                return v4l2_device.set_format(fmt);
            }

            idx++;
        }
        return false;
    };

    auto get_handler = [&v4l2_device](variant& value) ->
    bool
    {
        value = fmt_to_string(v4l2_device.get_format());
        return true;
    };

    for (const auto& fmt : formats)
    {
        list.emplace_back(fmt_to_string(fmt));
    }

    controls.emplace_back(control_parameter("Resolution"
                                            , control_type_t::list
                                            , list
                                            , fmt_to_string(v4l2_device.get_format())
                                            , custom_parameter
                                            , set_handler
                                            , get_handler
                                            )
                          );
}

static void fetch_controls(control_parameter_list_t& controls
                           , v4l2::v4l2_device& v4l2_device)
{
    auto v4l2_controls = v4l2_device.get_control_list();

    for (const auto& ctrl : v4l2_controls)
    {
        auto set_handler = [&v4l2_device, ctrl](variant& value) ->
        bool
        {
            switch(ctrl.type())
            {
                case v4l2::control_type_t::boolean:
                case v4l2::control_type_t::numeric:
                    return v4l2_device.set_control(ctrl.id
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
                            return v4l2_device.set_control(ctrl.id
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
            auto real_value = v4l2_device.get_control(ctrl.id
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
                                            , { visca::visca_pan_min, visca::visca_pan_max }
                                            , 0
                                            , custom_parameter
                                            , [&visca_device](variant& value) { return visca_device.set_pan(value); }
                                            , [&visca_device](variant& value) { std::int16_t pan = 0; if (!visca_device.get_pan(pan)) return false; value = pan; return true; }
                                            )
                          );

    controls.emplace_back(control_parameter("Visca tilt absolute"
                                            , control_type_t::direct
                                            , { visca::visca_tilt_min, visca::visca_tilt_max }
                                            , 0
                                            , custom_parameter
                                            , [&visca_device](variant& value) { return visca_device.set_tilt(value); }
                                            , [&visca_device](variant& value) { std::int16_t tilt = 0; if (!visca_device.get_tilt(tilt)) return false; value = tilt; return true; }
                                            )
                          );

    controls.emplace_back(control_parameter("Visca zoom absolute"
                                            , control_type_t::direct
                                            , { visca::visca_zoom_min, visca::visca_zoom_max }
                                            , 0u
                                            , custom_parameter
                                            , [&visca_device](variant& value) { return visca_device.set_zoom(value); }
                                            , [&visca_device](variant& value) { std::uint16_t zoom = 0; if (!visca_device.get_zoom(zoom)) return false; value = zoom; return true; }
                                            )
                          );

}

class camera_control_helper
{
    bool                    m_is_visca;
    uvc_camera_control      m_uvc_control;
    visca_camera_control    m_visca_control;
public:
    camera_control_helper(v4l2::v4l2_device& v4l2_device
                          , visca::visca_device& visca_device
                          , const std::string& mode)
        : m_is_visca(mode == "VISCA")
        , m_uvc_control(v4l2_device
                        , mode == "UVC_B")
        , m_visca_control(visca_device)
    {

    }

    i_camera_control& control()
    {
        return m_is_visca
                ? static_cast<i_camera_control&>(m_visca_control)
                : static_cast<i_camera_control&>(m_uvc_control);
    }
};

static void fetch_custom_parameters(control_parameter_list_t& controls
                                    , v4l2::v4l2_device& v4l2_device
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
                return control_helper.control().control_pan(value.get<std::int32_t>());
            }
            case custom_control_type_t::tilt:
            {
                return control_helper.control().control_tilt(value.get<std::int32_t>());
            }
            case custom_control_type_t::zoom:
            {
                return control_helper.control().control_zoom(value.get<std::int32_t>());
            }
            case custom_control_type_t::ptz:
            {
                return control_helper.control().set_preset(value.get<std::uint64_t>());
            }
        };

        return false;
    };


    auto get_handler_ptz = [&v4l2_device, &visca_device, &controls](variant& value) -> bool
    {

        variant mode;
        controls.get("Control mode", mode);

        camera_control_helper control_helper(v4l2_device
                                             , visca_device
                                             , mode);

        std::uint64_t ptz = 0;

        if (control_helper.control().get_preset(ptz))
        {
            value = ptz;
            return true;
        }

        return false;
    };

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

    controls.emplace_back(control_parameter("Preset"
                                            , control_type_t::direct
                                            , {} // { 0ul, 0xfffffffffffffffful }// { -1, 1 }
                                            , 0
                                            , custom_parameter
                                            , std::bind(set_handler, std::placeholders::_1, custom_control_type_t::ptz)
                                            , get_handler_ptz)
                          );
}

void format_form_frame_info(const v4l2::frame_info_t& frame_info, media_format_t& media_format)
{
    video::video_info_t video_info(utils::format_conversion::from_v4l2_video_format(frame_info.pixel_format)
                                   , { frame_info.size.width, frame_info.size.height}
                                   , frame_info.fps);

    media_format = media_format_t(video_info
                                  , 0);
}

static media_frame_ptr_t create_video_frame(v4l2::frame_t&& frame
                                            , frame_id_t frame_id)
{
    media_format_t media_format;
    format_form_frame_info(frame.frame_info
                           , media_format);

    return video::video_frame::create(media_format
                                      , media_buffer::create(std::move(frame.frame_data))
                                      , frame_id);
}

v4l2_input_media_device::v4l2_input_media_device(i_media_sink &media_sink
                                                 , std::uint32_t buffer_count)
    : m_frame_counter(0)
    , m_v4l2_device([&media_sink, this](v4l2::frame_t&& v4l2_frame) ->
        bool
        {
            auto frame = create_video_frame(std::move(v4l2_frame)
                                            , m_frame_counter);

            if (frame != nullptr)
            {
                m_frame_counter++;
                media_sink.on_frame(*frame);
            }

            return true;
        })
    , m_buffer_count(buffer_count)
{

}

bool v4l2_input_media_device::open(const std::string &uri)
{
    if (m_v4l2_device.open(uri
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
    return m_v4l2_device.close();
}

bool v4l2_input_media_device::is_open() const
{
    return m_v4l2_device.is_opened();
}

bool v4l2_input_media_device::is_established() const
{
    return m_v4l2_device.is_established();
}

media_format_list_t v4l2_input_media_device::streams() const
{
    media_format_list_t format_list;

    if (m_v4l2_device.is_opened())
    {
        media_format_t media_format;
        format_form_frame_info(m_v4l2_device.get_format()
                               , media_format);
        format_list.emplace_back(std::move(media_format));
    }

    return format_list;
}

const control_parameter_list_t& v4l2_input_media_device::controls() const
{
    return m_controls;
}

bool v4l2_input_media_device::set_control(const std::string& control_name
                                          , const variant& control_value)
{
    return m_controls.set(control_name
                          , control_value);
}

variant v4l2_input_media_device::get_control(const std::string& control_name
                                             , const variant& default_value) const
{
    variant result = default_value;

    m_controls.get(control_name
                   , result);

    return result;
}

}

}
