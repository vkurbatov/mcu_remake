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

std::string media_format_t::to_string() const
{
    static std::string names[] = { "audio", "video", "data" };
    return names[static_cast<std::uint32_t>(media_type)];
}

}

}
