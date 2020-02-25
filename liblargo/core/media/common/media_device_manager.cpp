#include "media_device_manager.h"
#include "libav_input_media_device.h"
#include "libav_output_media_device.h"
#include "v4l2_input_media_device.h"
#include "vnc_input_media_device.h"

namespace core
{

namespace media
{


static input_media_device_ptr_t create_input_device(const device_info_t& device_info
                                                    , i_media_sink& media_sink
                                                    , const manager_config_t& config)
{
    switch(device_info.type())
    {
        case device_type_t::camera:
            return input_media_device_ptr_t(new v4l2_input_media_device(media_sink
                                                                        , config.v4l2_buffers_count));
        break;
        case device_type_t::rtsp:
        case device_type_t::file:
        case device_type_t::rtmp:
            return input_media_device_ptr_t(new libav_input_media_device(media_sink));
        break;
        case device_type_t::vnc:
            return input_media_device_ptr_t(new vnc_input_media_device(media_sink
                                                                       , vnc::vnc_config_t(config.vnc_fps)));
        break;
    }
    return nullptr;
}

static input_media_device_ptr_t create_output_device(const device_info_t& device_info
                                                    , i_media_sink& media_sink
                                                    , const manager_config_t& config)
{
    switch(device_info.type())
    {
        case device_type_t::camera:
            return input_media_device_ptr_t(new v4l2_input_media_device(media_sink
                                                                        , config.v4l2_buffers_count));
        break;
        case device_type_t::rtsp:
        case device_type_t::file:
        case device_type_t::rtmp:
            return input_media_device_ptr_t(new libav_input_media_device(media_sink));
        break;
        case device_type_t::vnc:
            return input_media_device_ptr_t(new vnc_input_media_device(media_sink
                                                                       , vnc::vnc_config_t(config.vnc_fps)));
        break;
    }
    return nullptr;
}

class input_media_managed_device : virtual public i_input_media_managed_device
        , virtual public i_media_sink
{
    input_media_device_ptr_t    m_input_media_device;
    media_sink_ptr_t            m_media_sink;
    device_info_t               m_device_info;

public:
    input_media_managed_device();

    // i_media_control interface
public:
    const control_parameter_list_t &controls() const override;
    bool set_control(const std::string &control_name
                     , const variant &control_value) override;
    variant get_control(const std::string &control_name
                        , const variant &control_value) const override;

    // i_media_device interface
public:
    bool open(const std::string &uri) override;
    bool close() override;
    bool is_open() const override;
    bool is_established() const override;
    media_format_list_t streams() const override;

    // i_media_device_info interface
public:
    const device_info_t &device_info() const override
    {
        return m_device_info;
    }

    // i_media_sink interface
public:
    bool on_frame(const i_media_frame &frame) override
    {
        return m_media_sink != nullptr
                && m_media_sink->on_frame(frame);
    }
};

class output_media_managed_device : public i_output_media_managed_device
{
    output_media_device_ptr_t    m_output_media_device;
public:
    output_media_managed_device();

    // i_media_control interface
public:
    const control_parameter_list_t &controls() const override;
    bool set_control(const std::string &control_name, const variant &control_value) override;
    variant get_control(const std::string &control_name, const variant &control_value) const override;

    // i_media_device interface
public:
    bool open(const std::string &uri) override;
    bool close() override;
    bool is_open() const override;
    bool is_established() const override;
    media_format_list_t streams() const override;

    // i_media_sink interface
public:
    bool on_frame(const i_media_frame &frame) override;

    // i_media_device_info interface
public:
    const device_info_t &device_info() const override;
};
//--------------------------------------------------------------
manager_config_t::manager_config_t(uint32_t v4l2_buffers_count
                                   , std::uint32_t vnc_fps)
    : v4l2_buffers_count(v4l2_buffers_count)
    , vnc_fps(vnc_fps)
{

}
//--------------------------------------------------------------
media_device_manager::media_device_manager(const manager_config_t& config)
    : m_config(config)
{

}

device_info_list_t media_device_manager::device_info_list() const
{

}

bool media_device_manager::register_device(const device_info_t &device_info) const
{

}

bool media_device_manager::unregister_device(device_id_t device_id) const
{

}

input_media_managed_device_ptr_t media_device_manager::create_input_device(const device_info_t &device_info
                                                                           , media_sink_ptr_t sink)
{

}

output_media_managed_device_ptr_t media_device_manager::create_output_device(const device_info_t &device_info)
{

}

}

}
