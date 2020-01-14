#include "media_frame.h"
#include <numeric>
#include <cstring>

#include "media_plane.h"

namespace core
{

namespace media
{


bool media_frame::check_media_buffer(const media_format_t &media_format
                                     , const i_media_buffer &media_buffer)
{
    return media_format.is_encoded()
           || media_buffer.plane_sizes() == media_format.plane_sizes();
}

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

    for (std::uint32_t i = 0; i < m_media_buffer.plane_sizes().size(); i++)
    {
        plane_list.emplace_back(new media_plane(m_media_buffer
                                                , i));
    }

    return std::move(plane_list);
}

std::size_t media_frame::size() const
{
    return std::accumulate(m_media_buffer.plane_sizes().begin()
                           , m_media_buffer.plane_sizes().end()
                           , 0);
}

bool media_frame::is_valid() const
{
    return m_media_format.is_valid()
            && check_media_buffer(m_media_format
                                  , m_media_buffer);
}

}

}
