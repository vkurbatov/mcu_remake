#ifndef VIDEO_FRAME_BUFFERED_H
#define VIDEO_FRAME_BUFFERED_H

#include "video_frame.h"
#include "media/common/media_buffer.h"

namespace core
{

namespace media
{

namespace video
{

class video_frame_buffered : virtual public video_frame
{

    friend class video_frame_factory;

    media_buffer    m_media_buffer;
    video_frame_buffered(const video_format_t& video_format
                         , media::media_buffer&& media_buffer);

};

}

}

}

#endif // VIDEO_FRAME_BUFFERED_H
