#include "media_plane.h"
#include <numeric>

namespace core
{

namespace media
{

media_plane::media_plane(i_media_buffer &media_buffer
                         , std::uint32_t index)
    : m_media_buffer(media_buffer)
    , m_index(index)
{

}

const void *media_plane::data() const
{

    return m_index < m_media_buffer.planar_sizes().size()
            ? m_media_buffer.data(
                std::accumulate(m_media_buffer.planar_sizes().begin()
                              , m_media_buffer.planar_sizes().begin() + m_index
                              , 0))
            : nullptr;
}

std::size_t media_plane::size() const
{
    return m_index < m_media_buffer.planar_sizes().size()
            ? m_media_buffer.planar_sizes()[m_index]
            : 0;
}

}

}
