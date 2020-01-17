#include "media_plane.h"
#include <numeric>

namespace core
{

namespace media
{

media_plane::media_plane(media_buffer_ptr_t media_buffer
                         , std::uint32_t plane_id
                         , std::int32_t offset
                         , std::size_t size)
    : m_media_buffer(media_buffer)
    , m_plane_id(plane_id)
    , m_offset(offset)
    , m_size(size)
{

}

const void *media_plane::data() const
{
    return m_media_buffer->data(m_offset);
}

void *media_plane::data()
{
    return m_media_buffer->data(m_offset);
}

std::size_t media_plane::size() const
{
    return m_size;
}

uint32_t media_plane::plane_id() const
{
    return m_plane_id;
}

}

}
