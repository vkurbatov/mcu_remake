#include "media_frame.h"
#include <numeric>
#include <cstring>
#include <chrono>

#include "media_plane.h"
#include "media_buffer.h"

namespace core
{

namespace media
{


bool media_frame::check_media_buffer(const media_format_t &media_format
                                     , const i_media_buffer &media_buffer)
{
    return media_format.is_encoded()
           || media_buffer.size() == media_format.frame_size();
}

media_frame::media_frame(media_buffer_ptr_t media_buffer
                         , frame_id_t frame_id
                         , timestamp_t timestamp)
    : m_media_buffer(media_buffer)
    , m_frame_id(frame_id)
    , m_timestamp(timestamp)
{
    if (m_timestamp == 0)
    {
        m_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }
}

media_frame::media_frame(media_data_t &&media_data
                         , frame_id_t frame_id
                         , timestamp_t timestamp)
    : media_frame(media_buffer::create(std::move(media_data))
                  , frame_id
                  , timestamp)
{

}

media_frame::~media_frame(){}

media_plane_list_t media_frame::planes() const
{
    media_plane_list_t plane_list;

    if (m_media_buffer != nullptr)
    {
        if (media_format().is_encoded())
        {
            plane_list.emplace_back(new media_plane(m_media_buffer
                                                , 0
                                                , 0
                                                , m_media_buffer->size()));
        }
        else
        {
            auto i = 0;
            auto offset = 0;


            for (const auto& sz : media_format().plane_sizes())
            {
                plane_list.emplace_back(new media_plane(m_media_buffer
                                                        , i
                                                        , offset
                                                        , sz));
                i++;
                offset += sz;
            }
        }

    }

    return std::move(plane_list);
}

std::size_t media_frame::size() const
{
    return m_media_buffer->size();
}

bool media_frame::is_valid() const
{
    return m_media_buffer != nullptr
            && media_format().is_valid()
            && check_media_buffer(media_format()
                                  , *m_media_buffer);
}

frame_id_t media_frame::frame_id() const
{
    return m_frame_id;
}

timestamp_t media_frame::timestamp() const
{
    return m_timestamp;
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
