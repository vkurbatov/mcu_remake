#include "video_frame.h"
#include "media/common/media_frame.h"
#include "video_utils.h"

namespace core
{

namespace media
{

namespace video
{

media_frame_ptr_t video_frame::create(const media_format_t &media_format
                                      , media_buffer_ptr_t media_buffer
                                      , frame_id_t frame_id
                                      , timestamp_t timestamp)
{
    media_frame_ptr_t frame;

    if (media_buffer != nullptr && media_format.media_type == media_type_t::video)
    {
        frame.reset(new video_frame(media_format
                                    , media_buffer
                                    , frame_id
                                    , timestamp));
    }

    return frame;
}

media_frame_ptr_t video_frame::create(const media_format_t &media_format
                                      , media_data_t &&media_data
                                      , frame_id_t stream_id
                                      , timestamp_t timestamp)
{
    media_frame_ptr_t frame;

    if (media_format.media_type == media_type_t::video)
    {
        frame.reset(new video_frame(media_format
                                    , std::move(media_data)
                                    , stream_id
                                    , timestamp));
    }

    return frame;
}

video_frame::video_frame(const media_format_t& media_format
                         , media_buffer_ptr_t media_buffer
                         , frame_id_t frame_id
                         , timestamp_t timestamp)
    : media_frame(media_format
                  , media_buffer
                  , frame_id
                  , timestamp)
{

}

video_frame::video_frame(const media_format_t& media_format
                         , media_data_t &&media_data
                         , frame_id_t frame_id
                         , timestamp_t timestamp)
    : media_frame(media_format
                  , std::move(media_data)
                  , frame_id
                  , timestamp)
{

}

media_frame_ptr_t video_frame::clone() const
{
    return create(m_media_format
                  , m_media_buffer->clone()
                  , frame_id()
                  , timestamp());
}

void video_frame::clear()
{
    video_utils::blackout(*this);
}


}

}

}
