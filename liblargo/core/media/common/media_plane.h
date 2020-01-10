#ifndef MEDIA_PLANE_H
#define MEDIA_PLANE_H

#include "i_media_plane.h"
#include "i_media_buffer.h"

namespace core
{

namespace media
{

class media_plane : virtual public i_media_plane
{
    i_media_buffer&     m_media_buffer;
    std::uint32_t       m_index;

public:
    media_plane(i_media_buffer& media_buffer
                , std::uint32_t index);

    // i_media_plane interface
public:
    const void *data() const override;
    std::size_t size() const override;
};

}

}

#endif // MEDIA_PLANE_H
