#ifndef I_INPUT_DEVICE_H
#define I_INPUT_DEVICE_H

#include "device_info.h"

namespace core
{

namespace media
{

class i_input_device
{
public:
    virtual ~i_input_device(){}

    virtual bool open() = 0;
    virtual bool close() = 0;
    virtual bool is_open() const = 0;
    virtual bool is_established() const = 0;

    virtual const device_info_t& device_info() const = 0;

};

}

}

#endif // I_INPUT_DEVICE_H
