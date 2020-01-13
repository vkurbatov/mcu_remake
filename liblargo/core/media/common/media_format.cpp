#include "media_format.h"

namespace core
{

namespace media
{

media_format_t::media_format_t(media_type_t media_type)
    : media_type(media_type)
{

}

bool media_format_t::is_valid() const
{
    return is_encoded()
            || planes() > 0;
}

}

}
