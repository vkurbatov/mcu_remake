#ifndef VIDEO_FRAME_H
#define VIDEO_FRAME_H

#include "i_video_frame.h"
#include "media/common/media_frame.h"
#include "media/common/media_buffer.h"

namespace core
{

namespace media
{

namespace video
{

class video_frame : virtual public i_video_frame
        , public media_frame
{

public:
    static media_frame_ptr_t create(const media_format_t& media_format
                                    , media_buffer_ptr_t media_buffer
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


    video_frame(const media_format_t& media_format
                , media_buffer_ptr_t media_buffer = nullptr
                , frame_id_t frame_id = 0
                , timestamp_t timestamp = 0);

    video_frame(const media_format_t& media_format
                , media_data_t&& media_data
                , frame_id_t frame_id = 0
                , timestamp_t timestamp = 0);

    video_frame(const media_format_t& media_format
                , const void* data
                , std::size_t size
                , frame_id_t frame_id = 0
                , timestamp_t timestamp = 0);

    // i_media_frame interface
public:
    media_frame_ptr_t clone() const override;
    void clear() override;

};

}

}

}

#endif // VIDEO_FRAME_H
