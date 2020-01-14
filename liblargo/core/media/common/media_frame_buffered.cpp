#include "media_frame_buffered.h"

namespace core
{

namespace media
{

media_buffer media_frame_buffered::create_media_buffer(const media_format_t &media_format
                                                       , const void** slices
                                                       , std::size_t size)
{
    return media_format.is_encoded()
            ? media_buffer(slices == nullptr
                           ? nullptr
                           : *slices, size)
            : media_buffer(slices
                           , media_format.plane_sizes());
}

media_frame_buffered::media_frame_buffered(const media_format_t &media_format
                                           , media::media_buffer &&media_buffer)
    : m_media_buffer(std::move(media_frame::check_media_buffer(media_format
                                                                 , media_buffer)
                                  ? media_buffer
                                  : create_media_buffer(media_format)))
    , m_media_frame(media_format
                    , m_media_buffer)
{

}

bool media_frame_buffered::swap_buffer(media::media_buffer &&media_buffer)
{
    return media_frame::check_media_buffer(media_format()
                                           , media_buffer)
            && m_media_buffer.swap(std::move(media_buffer));
}

media_buffer media_frame_buffered::release()
{
    return std::move(m_media_buffer);
}

const media_format_t &media_frame_buffered::media_format() const
{
    return m_media_frame.media_format();
}

media_plane_list_t media_frame_buffered::planes() const
{
    return m_media_frame.planes();
}

std::size_t media_frame_buffered::size() const
{
    return m_media_frame.size();
}

bool media_frame_buffered::is_valid() const
{
    return m_media_frame.is_valid();
}

}

}
