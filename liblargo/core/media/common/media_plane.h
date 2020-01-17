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
    std::shared_ptr<i_media_buffer>     m_media_buffer;
    std::uint32_t                       m_plane_id;
    std::int32_t                        m_offset;
    std::size_t                         m_size;

public:
    media_plane(media_buffer_ptr_t media_buffer
                , std::uint32_t plane_id
                , std::int32_t offset
                , std::size_t size);

    // i_media_plane interface
public:
    const void *data() const override;
    void *data() override;
    std::size_t size() const override;
    std::uint32_t plane_id() const override;
};

}

}

#endif // MEDIA_PLANE_H
