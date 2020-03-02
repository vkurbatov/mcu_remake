#include "media_frame.h"
#include <numeric>
#include <cstring>
#include <chrono>

#include "media/video/video_frame.h"
#include "media/audio/audio_frame.h"

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

timestamp_t media_frame::now_timestamp()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

media_frame_ptr_t media_frame::create(const media_format_t &media_format
                                      , media_buffer_ptr_t media_buffer
                                      , frame_id_t frame_id
                                      , timestamp_t timestamp)
{
    switch(media_format.media_type)
    {
        case media_type_t::audio:
            return audio::audio_frame::create(media_format
                                              , media_buffer
                                              , frame_id
                                              , timestamp);
        break;
        case media_type_t::video:
            return video::video_frame::create(media_format
                                              , media_buffer
                                              , frame_id
                                              , timestamp);
        break;
    }
}

media_frame_ptr_t media_frame::create(const media_format_t &media_format
                                      , media_data_t &&media_data
                                      , frame_id_t frame_id
                                      , timestamp_t timestamp)
{
    switch(media_format.media_type)
    {
        case media_type_t::audio:
            return audio::audio_frame::create(media_format
                                              , std::move(media_data)
                                              , frame_id
                                              , timestamp);
        break;
        case media_type_t::video:
            return video::video_frame::create(media_format
                                              , std::move(media_data)
                                              , frame_id
                                              , timestamp);
        break;
    }
}

media_frame::media_frame(const media_format_t& media_format
                         , media_buffer_ptr_t media_buffer
                         , frame_id_t frame_id
                         , timestamp_t timestamp)
    : m_media_format(media_format)
    , m_media_buffer(media_buffer)
    , m_frame_id(frame_id)
    , m_timestamp(timestamp)
    , m_attributes(frame_attributes_t::undefined)
{
    if (m_timestamp == 0)
    {
        m_timestamp = now_timestamp();
    }
}

media_frame::media_frame(const media_format_t& media_format
                         , media_data_t &&media_data
                         , frame_id_t frame_id
                         , timestamp_t timestamp)
    : media_frame(media_format
                  , media_buffer::create(std::move(media_data))
                  , frame_id
                  , timestamp)
{

}

media_frame::~media_frame(){}

const media_format_t &media_frame::media_format() const
{
    return m_media_format;
}

media_format_t &media_frame::media_format()
{
    return m_media_format;
}

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

const void *media_frame::data(std::int32_t offset) const
{
    return m_media_buffer != nullptr
            ? m_media_buffer->data(offset)
            : nullptr;
}

std::size_t media_frame::size() const
{
    return m_media_buffer != nullptr
            ? m_media_buffer->size()
            : 0;
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

frame_attributes_t media_frame::frame_attributes() const
{
    return m_attributes;
}

void media_frame::set_frame_id(frame_id_t frame_id)
{
    m_frame_id = frame_id;
}

void media_frame::set_timestamp(timestamp_t timestamp)
{
    m_timestamp = timestamp;
}

void media_frame::set_attributes(frame_attributes_t attributes)
{
    m_attributes = attributes;
}

void media_frame::swap(media_buffer_ptr_t &&media_buffer)
{
    m_media_buffer.swap(media_buffer);
}

media_buffer_ptr_t media_frame::release()
{
    return std::move(m_media_buffer);
}

void media_frame::clear()
{
    if (!media_format().is_encoded()
            && m_media_buffer != nullptr
            && m_media_buffer->size()>0)
    {
        std::memset(m_media_buffer->data()
                    , 0
                    , m_media_buffer->size());
    }
}

}

}
