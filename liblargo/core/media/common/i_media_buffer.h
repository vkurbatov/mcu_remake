#ifndef I_MEDIA_BUFFER_H
#define I_MEDIA_BUFFER_H

#include "media_format.h"

#include <cstdint>
#include <vector>
#include <memory>

namespace core
{

namespace media
{

typedef std::vector<std::uint8_t> media_data_t;

class i_media_buffer
{
public:
    virtual ~i_media_buffer() {}
    virtual const void* data(std::int32_t offset = 0) const = 0;
    virtual void* data(std::int32_t offset = 0) = 0;
    virtual const plane_sizes_t& plane_sizes() const = 0;
};

class i_swapped_media_buffer : virtual public i_media_buffer
{
public:
    virtual ~i_swapped_media_buffer() {}
    virtual bool swap(media_data_t&& media_data
              , plane_sizes_t&& plane_sizes) = 0;
    virtual media_data_t release() = 0;

};

typedef std::unique_ptr<i_media_buffer> media_buffer_ptr_t;

}

}

#endif // I_MEDIA_BUFFER_H
