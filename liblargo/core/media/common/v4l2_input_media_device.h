#ifndef V4L2_INPUT_MEDIA_DEVICE_H
#define V4L2_INPUT_MEDIA_DEVICE_H

#include "i_media_sink.h"
#include "i_media_device.h"
#include "i_media_control.h"

#include "media/common/v4l2/v4l2_device.h"
#include "media/common/visca/visca_device.h"

namespace core
{

namespace media
{

typedef std::unique_ptr<v4l2::v4l2_device> v4l2_device_ptr_t;

class v4l2_input_media_device : virtual public i_media_device,
        virtual public i_media_control

{
    v4l2_device_ptr_t           m_v4l2_device;
    visca::visca_device         m_visca_device;

    std::uint32_t               m_buffer_count;

    control_parameter_list_t    m_controls;

public:
    v4l2_input_media_device(i_media_sink& media_sink
                            , std::uint32_t buffer_count = 2);

    // i_media_device interface
public:
    bool open(const std::string &uri) override;
    bool close() override;
    bool is_open() const override;
    bool is_established() const override;

    // i_media_control interface
public:
    const control_parameter_list_t& controls() const override;

    bool set_control(const std::string &control_name
                     , const variant control_value) override;

    variant get_control(const std::string &control_name
                        , const variant default_value = {}) const override;
};

}

}

#endif // V4L2_INPUT_MEDIA_DEVICE_H
