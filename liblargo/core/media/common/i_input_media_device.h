#ifndef I_INPUT_MEDIA_DEVICE_H
#define I_INPUT_MEDIA_DEVICE_H

#include "i_media_device.h"
#include "i_media_control.h"

namespace core
{

namespace media
{

class i_input_media_device : virtual public i_media_device
        , virtual public i_media_control
{
public:
    virtual ~i_input_media_device(){}
};

typedef std::shared_ptr<i_input_media_device> input_media_device_ptr_t;

}

}

#endif // I_INPUT_MEDIA_DEVICE_H
