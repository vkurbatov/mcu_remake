#ifndef I_MEDIA_DEVICE_H
#define I_MEDIA_DEVICE_H

#include "device_info.h"
#include "media_format.h"

namespace core
{

namespace media
{

class i_media_device
{
public:
    virtual ~i_media_device(){}

    virtual bool open(const std::string& uri = {}) = 0;
    virtual bool close() = 0;
    virtual bool is_open() const = 0;
    virtual bool is_established() const = 0;

    virtual media_format_list_t streams() const = 0;
    // virtual const device_info_t& device_info() const = 0;
};

}

}

#endif // I_MEDIA_DEVICE_H
