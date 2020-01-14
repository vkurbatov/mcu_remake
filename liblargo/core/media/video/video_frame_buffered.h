#ifndef VIDEO_FRAME_BUFFERED_H
#define VIDEO_FRAME_BUFFERED_H

#include "i_video_frame.h"
#include "media/common/media_frame_buffered.h"

namespace core
{

namespace media
{

namespace video
{

class video_frame_buffered : virtual public i_video_frame
        , public media_frame_buffered
{

    friend class video_frame_factory;

    video_format_t      m_video_format;
public:
    video_frame_buffered(const video_format_t& video_format
                         , media::media_buffer&& media_buffer);


    // i_video_frame interface
public:
    const video_format_t &video_format() const override;
};

}

}

}

#endif // VIDEO_FRAME_BUFFERED_H
