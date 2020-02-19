#include "media_format.h"

namespace core
{

namespace media
{

media_format_t::media_format_t(media_type_t media_type
                               , stream_id_t stream_id)
    : media_type(media_type)
    , stream_id(stream_id)
{

}

bool media_format_t::is_valid() const
{
    return is_encoded()
            || planes() > 0;
}

bool media_format_t::operator ==(const media_format_t &media_format)
{
    return media_type == media_format.media_type
            && extra_data.size() == media_format.extra_data.size();
}

bool media_format_t::operator !=(const media_format_t &media_format)
{
    return !operator == (media_format);
}

std::string media_format_t::to_string() const
{
    static std::string names[] = { "audio", "video", "data" };
    return names[static_cast<std::uint32_t>(media_type)];
}

external_media_info_t::external_media_info_t(void *data
                                             , std::size_t size
                                             , const std::string &specific_options)
    : data(data)
    , size(size)
    , specific_options(specific_options)
{

}

}

}
