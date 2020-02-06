#ifndef DEVICE_INFO_H
#define DEVICE_INFO_H

#include <string>

namespace core
{

namespace media
{

enum class device_class_t
{
    input_device,
    output_device
};

enum class device_type_t
{
    unknown,
    audio,
    video
};

struct device_info_t
{
    device_class_t device_class;
    device_type_t device_type;
    std::string name;
    std::string description;
    std::string uri;

    device_info_t(device_class_t device_class
                  , device_type_t device_type
                  , std::string name
                  , std::string description
                  , std::string uri);
};

}

}

#endif // DEVICE_INFO_H
