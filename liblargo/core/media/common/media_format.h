#ifndef MEDIA_FORMAT_H
#define MEDIA_FORMAT_H

#include <cstdint>

namespace core
{

namespace media
{

enum class media_type_t
{
    audio,
    video,
    data
};

struct media_format_t
{
    media_type_t        media_type;
};

}

}

#endif // MEDIA_INFO_H
