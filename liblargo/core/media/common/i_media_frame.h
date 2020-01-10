#ifndef I_MEDIA_FRAME_H
#define I_MEDIA_FRAME_H

#include <cstdint>

namespace core
{

namespace media
{

enum class media_type_t;

class i_media_frame
{
public:
    virtual ~i_media_frame() {}
    virtual const media_type_t& media_type() const = 0;
    virtual const void* data() const = 0;
    virtual std::size_t size() const = 0;
};

}

}

#endif // I_MEDIA_FRAME_H
