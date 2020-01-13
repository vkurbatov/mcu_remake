#include "media_frame.h"
#include <numeric>
#include <cstring>

#include "media_plane.h"

namespace core
{

namespace media
{

media_frame::media_frame(const media_format_t &media_format
                         , i_media_buffer &media_buffer)
    : m_media_format(media_format)
    , m_media_buffer(media_buffer)
{

}

const media_format_t &media_frame::media_format() const
{
    return m_media_format;
}

media_plane_list_t media_frame::planes() const
{
    media_plane_list_t plane_list;

    for (std::uint32_t i = 0; i < m_media_buffer.planar_sizes().size(); i++)
    {
        plane_list.emplace_back(new media_plane(m_media_buffer
                                                , i));
    }

    return std::move(plane_list);
}

std::size_t media_frame::size() const
{
    return std::accumulate(m_media_buffer.planar_sizes().begin()
                           , m_media_buffer.planar_sizes().end()
                           , 0);
}

bool media_frame::is_valid() const
{
    return m_media_format.is_valid()
            && (m_media_format.is_encoded()
                || m_media_format.frame_size() == size());
}

}

}
