#ifndef MEDIA_CONTROL_H
#define MEDIA_CONTROL_H

#include "media/common/base/variant.h"


namespace core
{

namespace media
{

using variant_type_t = base::variant_type_t;
using variant = base::variant;

typedef std::vector<variant> variant_list_t;

enum class control_type_t
{
    number,
    real,
    string,
    boolean,
};



struct control_parameter_t
{
    variant_list_t  permission_list;

};

}

}

#endif // MEDIA_CONTROL_FORMAT_H
