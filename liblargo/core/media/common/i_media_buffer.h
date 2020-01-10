#ifndef I_MEDIA_BUFFER_H
#define I_MEDIA_BUFFER_H

#include <cstdint>
#include <vector>

namespace core
{

namespace media
{

typedef std::vector<std::size_t> planar_sizes_t;

class i_media_buffer
{
public:
    virtual ~i_media_buffer() {}
    virtual const void* data(std::int32_t offset = 0) const = 0;
    virtual const planar_sizes_t& planar_sizes() const = 0;
};

}

}

#endif // I_MEDIA_BUFFER_H
