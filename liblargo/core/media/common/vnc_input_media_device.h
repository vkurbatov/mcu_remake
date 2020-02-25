#ifndef VNC_INPUT_MEDIA_DEVICE_H
#define VNC_INPUT_MEDIA_DEVICE_H

#include "i_media_sink.h"
#include "i_input_media_device.h"

#include "media/common/vnc/vnc_device.h"

namespace core
{

namespace media
{

// typedef std::unique_ptr<vnc::vnc_device> vnc_device_ptr_t;

class vnc_input_media_device : virtual public i_input_media_device
{
    std::uint32_t               m_frame_counter;
    vnc::vnc_device             m_vnc_device;
    control_parameter_list_t    m_controls;
    vnc::frame_t                m_frame_info;

public:
    vnc_input_media_device(i_media_sink& media_sink
                           , const vnc::vnc_config_t& vnc_config = {});

    // i_media_device interface
public:
    bool open(const std::string &uri) override;
    bool close() override;
    bool is_open() const override;
    bool is_established() const override;
    media_format_list_t streams() const override;

    // i_media_control interface
public:
    const control_parameter_list_t& controls() const override;

    bool set_control(const std::string &control_name
                     , const variant& control_value) override;

    variant get_control(const std::string &control_name
                        , const variant& default_value = {}) const override;
};

}

}

#endif // VNC_INPUT_MEDIA_DEVICE_H
