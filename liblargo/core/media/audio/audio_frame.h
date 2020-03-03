#ifndef AUDIO_FRAME_H
#define AUDIO_FRAME_H

#include "media/common/media_frame.h"

namespace core
{

namespace media
{

namespace audio
{

class audio_frame : public media_frame
{
public:
    static media_frame_ptr_t create(const media_format_t& media_format
                                    , media_buffer_ptr_t media_buffer = nullptr
                                    , frame_id_t frame_id = 0
                                    , timestamp_t timestamp = 0);

    static media_frame_ptr_t create(const media_format_t& media_format
                                    , media_data_t&& media_data
                                    , frame_id_t frame_id = 0
                                    , timestamp_t timestamp = 0);

    static media_frame_ptr_t create(const media_format_t& media_format
                                    , const void* data
                                    , std::size_t size
                                    , frame_id_t frame_id = 0
                                    , timestamp_t timestamp = 0);

    audio_frame(const media_format_t& media_format
                , media_buffer_ptr_t media_buffer = nullptr
                , frame_id_t frame_id = 0
                , timestamp_t timestamp = 0);

    audio_frame(const media_format_t& media_format
                , media_data_t&& media_data
                , frame_id_t frame_id = 0
                , timestamp_t timestamp = 0);

    audio_frame(const media_format_t& media_format
                , const void* data
                , std::size_t size
                , frame_id_t frame_id = 0
                , timestamp_t timestamp = 0);

    // i_media_frame interface
public:
    media_frame_ptr_t clone() const override;
};

}

}

}


#endif // AUDIO_FRAME_H
