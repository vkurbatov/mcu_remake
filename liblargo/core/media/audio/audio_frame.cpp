#include "audio_frame.h"

#include <cstring>

namespace core
{

namespace media
{

namespace audio
{

media_frame_ptr_t audio_frame::create(const media_format_t &media_format
                                        , media_buffer_ptr_t media_buffer
                                        , frame_id_t frame_id
                                        , timestamp_t timestamp)
{
    media_frame_ptr_t frame;

    if (media_buffer != nullptr && media_format.media_type == media_type_t::audio)
    {
        frame.reset(new audio_frame(media_format
                                    , media_buffer
                                    , frame_id
                                    , timestamp));
    }

    return frame;
}

media_frame_ptr_t audio_frame::create(const media_format_t &media_format
                                        , media_data_t &&media_data
                                        , frame_id_t frame_id
                                        , timestamp_t timestamp)
{
    media_frame_ptr_t frame;

    if (media_format.media_type == media_type_t::audio)
    {
        frame.reset(new audio_frame(media_format
                                    , std::move(media_data)
                                    , frame_id
                                    , timestamp));
    }

    return frame;
}

media_frame_ptr_t audio_frame::create(const media_format_t &media_format
                                      , const void *data
                                      , std::size_t size
                                      , frame_id_t frame_id
                                      , timestamp_t timestamp)
{
    media_frame_ptr_t frame;

    if (media_format.media_type == media_type_t::audio)
    {
        frame.reset(new audio_frame(media_format
                                    , data
                                    , size
                                    , frame_id
                                    , timestamp));
    }

    return frame;
}

audio_frame::audio_frame(const media_format_t &media_format
                         , media_buffer_ptr_t media_buffer
                         , frame_id_t frame_id
                         , timestamp_t timestamp)
    : media_frame(media_format
                  , std::move(media_buffer)
                  , frame_id
                  , timestamp)
{

}

audio_frame::audio_frame(const media_format_t &media_format
                         , media_data_t &&media_data
                         , frame_id_t frame_id
                         , timestamp_t timestamp)
    : media_frame(media_format
                  , std::move(media_data)
                  , frame_id
                  , timestamp)
{

}

audio_frame::audio_frame(const media_format_t &media_format
                         , const void *data
                         , std::size_t size
                         , frame_id_t frame_id
                         , timestamp_t timestamp)
    : media_frame(media_format
                  , data
                  , size
                  , frame_id
                  , timestamp)
{

}

media_frame_ptr_t audio_frame::clone() const
{
    return create(m_media_format
                  , m_media_buffer->clone()
                  , m_frame_id
                  , m_timestamp);
}

}

}

}
