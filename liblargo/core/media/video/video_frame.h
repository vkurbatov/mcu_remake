#ifndef VIDEO_FRAME_H
#define VIDEO_FRAME_H

#include "i_video_frame.h"
#include "media/common/media_frame.h"

namespace core
{

namespace media
{

namespace video
{

class video_frame : virtual public i_video_frame
{
    video_format_t      m_video_format;
    media_frame         m_media_frame;
public:
    video_frame(const video_format_t& video_format
                , i_media_buffer& media_buffer);

    // i_media_frame interface
public:
    const media_format_t &media_format() const override;
    media_plane_list_t planes() const override;

    // i_video_frame interface
public:
    const video_format_t &video_format() const override;

};

}

}

}

#endif // VIDEO_FRAME_H
