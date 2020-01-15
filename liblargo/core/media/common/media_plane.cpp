#include "media_plane.h"
#include <numeric>

namespace core
{

namespace media
{

media_plane::media_plane(media_buffer_ptr_t media_buffer
                         , std::uint32_t plane_id)
    : m_media_buffer(media_buffer)
    , m_plane_id(plane_id)
{

}

const void *media_plane::data() const
{
    auto media_buffer = m_media_buffer.lock();

    return media_buffer != nullptr && m_plane_id < media_buffer->plane_sizes().size()
            ? media_buffer->data(
                std::accumulate(media_buffer->plane_sizes().begin()
                              , media_buffer->plane_sizes().begin() + m_plane_id
                              , 0))
            : nullptr;

}

void *media_plane::data()
{
    auto media_buffer = m_media_buffer.lock();

    return media_buffer != nullptr && m_plane_id < media_buffer->plane_sizes().size()
            ? media_buffer->data(
                std::accumulate(media_buffer->plane_sizes().begin()
                              , media_buffer->plane_sizes().begin() + m_plane_id
                              , 0))
            : nullptr;
}

std::size_t media_plane::size() const
{
    auto media_buffer = m_media_buffer.lock();

    return media_buffer != nullptr && m_plane_id < media_buffer->plane_sizes().size()
            ? media_buffer->plane_sizes()[m_plane_id]
              : 0;
}

uint32_t media_plane::plane_id() const
{
    return m_plane_id;
}

}

}
