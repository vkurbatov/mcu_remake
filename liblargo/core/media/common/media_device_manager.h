#ifndef MEDIA_DEVICE_MANAGER_H
#define MEDIA_DEVICE_MANAGER_H

#include "i_media_device_manager.h"

namespace core
{

namespace media
{

struct manager_config_t
{
    std::uint32_t   v4l2_buffers_count;
    std::uint32_t   vnc_fps;
    manager_config_t(std::uint32_t v4l2_buffers_count = 2
                    , std::uint32_t vnc_fps = 25);
};

class media_device_manager : virtual public i_media_device_manager
{
    device_info_list_t      m_custom_device_list;
    manager_config_t        m_config;

public:
    media_device_manager(const manager_config_t& config={});

    // i_media_device_manager interface
public:
    device_info_list_t device_info_list() const override;
    bool register_device(const device_info_t &device_info) const override;
    bool unregister_device(device_id_t device_id) const override;

    input_media_managed_device_ptr_t create_input_device(const device_info_t &device_info
                                                         , media_sink_ptr_t sink) override;
    output_media_managed_device_ptr_t create_output_device(const device_info_t &device_info) override;
};

}

}

#endif // MEDIA_DEVICE_MANAGER_H
