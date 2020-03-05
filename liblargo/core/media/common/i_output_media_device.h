#ifndef I_OUTPUT_MEDIA_DEVICE_H
#define I_OUTPUT_MEDIA_DEVICE_H

#include "i_media_device_control.h"
#include "i_media_sink.h"

namespace core
{

namespace media
{

class i_output_media_device : virtual public i_media_device_control
        , virtual public i_media_sink
{
public:
    virtual ~i_output_media_device(){}
};

typedef std::shared_ptr<i_output_media_device> output_media_device_ptr_t;

}

}


#endif // I_OUTPUT_MEDIA_DEVICE_H
