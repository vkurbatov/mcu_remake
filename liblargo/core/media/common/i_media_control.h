#ifndef I_MEDIA_CONTROL_H
#define I_MEDIA_CONTROL_H

#include "media_control_parameter.h"

namespace core
{

namespace media
{

class i_media_control
{

public:
    virtual ~i_media_control() {}

    virtual const control_parameter_list_t& controls() const = 0;

    virtual bool set_control(const std::string& control_name
                             , const variant control_value) = 0;


    virtual variant get_control(const std::string& control_name
                                , const variant control_value = {}) const = 0;

};

}

}

#endif // I_MEDIA_CONTROL_H
