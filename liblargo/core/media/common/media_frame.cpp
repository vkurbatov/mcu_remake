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

media_frame::media_frame(media_buffer_ptr_t media_buffer)
    : m_media_buffer(media_buffer)
{

}

media_plane_list_t media_frame::planes() const
{
    media_plane_list_t plane_list;

    if (m_media_buffer != nullptr)
    {
        for (std::uint32_t i = 0; i < m_media_buffer->plane_sizes().size(); i++)
        {
            plane_list.emplace_back(new media_plane(m_media_buffer
                                                    , i));
        }
    }

    return std::move(plane_list);
}

std::size_t media_frame::size() const
{
    return m_media_buffer != nullptr
            ? std::accumulate(m_media_buffer->plane_sizes().begin()
                           , m_media_buffer->plane_sizes().end()
                           , 0)
            : 0;
}

bool media_frame::is_valid() const
{
    return m_media_buffer != nullptr
            && media_format().is_valid()
            && check_media_buffer(media_format()
                                  , *m_media_buffer);
}

void media_frame::swap(media_buffer_ptr_t &&media_buffer)
{
    m_media_buffer.swap(media_buffer);
}

media_buffer_ptr_t media_frame::release()
{
    return std::move(m_media_buffer);
}

}

}