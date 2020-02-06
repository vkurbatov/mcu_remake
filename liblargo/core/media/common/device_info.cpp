#include "device_info.h"


namespace core
{

namespace media
{

device_info_t::device_info_t(device_class_t device_class
                            , device_type_t device_type
                            , std::string name
                            , std::string description
                            , std::string uri)
    : device_class(device_class)
    , device_type(device_type)
    , name(name)
    , description(description)
    , uri(uri)
{

}

}

}
