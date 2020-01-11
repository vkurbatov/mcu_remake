#ifndef I_MEDIA_PLANE_H
#define I_MEDIA_PLANE_H

#include <cstdint>

namespace core
{

namespace media
{

class i_media_plane
{
public:
    virtual ~i_media_plane() {}
    virtual const void* data() const = 0;
    virtual void* data() = 0;
    virtual std::size_t size() const = 0;
    virtual std::uint32_t plane_id() const = 0;
};

}

}

#endif // I_MEDIA_PLANE_H
