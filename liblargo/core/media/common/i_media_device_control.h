#ifndef I_MEDIA_DEVICE_CONTROL_H
#define I_MEDIA_DEVICE_CONTROL_H

#include <cstdint>
#include "media_control.h"

namespace core
{

namespace media
{

class i_media_device_control
{

public:
    virtual ~i_media_device_control() {}

    virtual media_control_list controls() const = 0;

    virtual bool set_control(const std::string& control_name
                             , const std::string& control_value) const = 0;

    virtual bool set_control(const std::string& control_name
                             , control_value_t control_value) const = 0;

};

}

}

#endif // I_MEDIA_CONTROL_H
