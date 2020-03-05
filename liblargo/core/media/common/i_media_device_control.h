#ifndef I_MEDIA_DEVICE_CONTROL_H
#define I_MEDIA_DEVICE_CONTROL_H

#include "i_media_device.h"
#include "i_media_control.h"

namespace core
{

namespace media
{

class i_media_device_control : virtual public i_media_device
        , virtual public i_media_control
{
public:
    virtual ~i_media_device_control(){}
};

typedef std::shared_ptr<i_media_device_control> media_device_control_ptr_t;

}

}


#endif // I_MEDIA_DEVICE_CONTROL_H
