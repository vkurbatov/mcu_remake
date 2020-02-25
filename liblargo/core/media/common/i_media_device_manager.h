#ifndef I_MEDIA_DEVICE_MANAGER_H
#define I_MEDIA_DEVICE_MANAGER_H

#include "i_input_media_device.h"
#include "i_output_media_device.h"

#include "device_info.h"

#include <vector>

namespace core
{

namespace media
{

class i_media_device_info
{
public:
    virtual ~i_media_device_info(){}
    virtual const device_info_t& device_info() const = 0;
};

class i_input_media_managed_device : virtual public i_input_media_device
        , virtual public i_media_device_info
{
public:
    virtual ~i_input_media_managed_device(){}
};

typedef std::shared_ptr<i_input_media_managed_device> input_media_managed_device_ptr_t;

class i_output_media_managed_device : virtual public i_output_media_device
        , virtual public i_media_device_info
{
public:
    virtual ~i_output_media_managed_device(){}
};

typedef std::shared_ptr<i_output_media_managed_device> output_media_managed_device_ptr_t;

typedef std::vector<device_info_t> device_info_list_t;

class i_media_device_manager
{
public:
    virtual ~i_media_device_manager() {}
    virtual device_info_list_t device_info_list() const = 0;

    virtual bool register_device(const device_info_t& device_info) const = 0;
    virtual bool unregister_device(device_id_t device_id) const = 0;

    virtual input_media_managed_device_ptr_t create_input_device(const device_info_t& device_info
                                                         , media_sink_ptr_t sink) = 0;
    virtual output_media_managed_device_ptr_t create_output_device(const device_info_t& device_info) = 0;

};

}

}

#endif // I_MEDIA_DEVICE_MANAGER_H
