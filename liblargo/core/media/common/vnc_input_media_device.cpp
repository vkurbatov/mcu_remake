#include "vnc_input_media_device.h"
#include "media/video/video_frame.h"

namespace core
{

namespace media
{

static video::pixel_format_t bbp_to_format(std::uint16_t bpp)
{
    switch (bpp)
    {
        case 8:
            return video::pixel_format_t::rgb8;
        break;
        case 15:
            return video::pixel_format_t::rgb555;
        break;
        case 16:
            return video::pixel_format_t::rgb565;
        break;
        case 24:
            return video::pixel_format_t::rgb24;
        break;
        case 32:
            return video::pixel_format_t::rgb32;
        break;
    }

    return video::pixel_format_t::unknown;
}

media_format_ptr_t format_form_frame_info(const vnc::frame_t& frame)
{
    return video::video_format_t::create(bbp_to_format(frame.bpp)
                                         , { frame.frame_size.width, frame.frame_size.height}
                                         , frame.fps
                                         , 0);
}

vnc_input_media_device::vnc_input_media_device(i_media_sink &media_sink
                                               , const vnc::vnc_config_t& vnc_config)
    : m_frame_counter(0)
    , m_vnc_device([&media_sink, this](vnc::frame_t&& vnc_frame)
    {
        m_frame_info.bpp = vnc_frame.bpp;
        m_frame_info.frame_size = vnc_frame.frame_size;
        m_frame_info.fps = vnc_frame.fps;

        if (!vnc_frame.frame_data.empty())
        {
            video::video_format_t video_format(bbp_to_format(vnc_frame.bpp)
                                               , { vnc_frame.frame_size.width, vnc_frame.frame_size.height}
                                               , vnc_frame.fps
                                               , m_frame_counter);

            if (video_format.pixel_format != video::pixel_format_t::unknown
                    && video_format.size.size() <= vnc_frame.frame_data.size())
            {
                auto frame = video::video_frame::create(video_format
                                                        , std::move(vnc_frame.frame_data));

                if (frame != nullptr)
                {
                    m_frame_counter++;
                    media_sink.on_frame(*frame);
                }
            }
        }

        return true;
    }
    , vnc_config)
{
    auto set_handler = [this](variant& value)
    {
        auto vnc_config = m_vnc_device.config();
        vnc_config.fps = value;
        return m_vnc_device.set_config(vnc_config);
    };

    auto get_handler = [this](variant& value)
    {
        value = m_vnc_device.config().fps;
        return true;
    };

    m_controls.emplace_back(control_parameter("fps"
                                              , control_type_t::list
                                              , { 5, 10, 15, 20, 25, 30 }
                                              , vnc::default_fps
                                              , custom_parameter
                                              , set_handler
                                              , get_handler
                                              )
                            );
}

bool vnc_input_media_device::open(const std::string &uri)
{
    return m_vnc_device.open(uri);
}

bool vnc_input_media_device::close()
{
    return m_vnc_device.close();
}

bool vnc_input_media_device::is_open() const
{
    return m_vnc_device.is_opened();
}

bool vnc_input_media_device::is_established() const
{
    return m_vnc_device.is_established();
}

media_format_list_t vnc_input_media_device::streams() const
{
    media_format_list_t format_list;

    if (m_vnc_device.is_opened())
    {
        format_list.emplace_back(format_form_frame_info(m_frame_info));
    }

    return format_list;
}

const control_parameter_list_t &vnc_input_media_device::controls() const
{
    return m_controls;
}

bool vnc_input_media_device::set_control(const std::string &control_name
                                         , const variant& control_value)
{
    return m_controls.set(control_name
                          , control_value);
}

variant vnc_input_media_device::get_control(const std::string &control_name
                                            , const variant& default_value) const
{
    auto result = default_value;

    m_controls.get(control_name
                          , result);
    return result;
}



}

}
