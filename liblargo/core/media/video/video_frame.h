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
    video_format_t      m_video_format;

public:
    static media_frame_ptr_t create(const media_format_t& media_format
                                    , media_buffer_ptr_t media_buffer);

    video_frame(const video_format_t& video_format
                , media_buffer_ptr_t media_buffer = nullptr);

    // i_media_frame interface
public:
    const media_format_t &media_format() const override;

    // i_video_frame interface
public:
    const video_format_t &video_format() const override;
};

}

}

}

#endif // VIDEO_FRAME_H
