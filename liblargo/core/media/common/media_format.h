#ifndef MEDIA_FORMAT_H
#define MEDIA_FORMAT_H

#include <cstdint>
#include <vector>

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

typedef std::vector<std::size_t> plane_sizes_t;

struct media_format_t
{
    media_type_t        media_type;
    media_format_t(media_type_t media_type);

    virtual bool is_encoded() const = 0;
    virtual bool is_planar() const = 0;
    virtual std::size_t frame_size() const = 0;
    virtual std::size_t planes() const = 0;
    virtual plane_sizes_t plane_sizes() const = 0;
    virtual bool is_valid() const;

};

}

}

#endif // MEDIA_INFO_H
