#ifndef I_VIDEO_FRAME_H
#define I_VIDEO_FRAME_H

#include "media/common/i_media_frame.h"
#include "video_format.h"

namespace core
{

namespace media
{

namespace video
{

class i_video_frame : virtual public i_media_frame
{
public:
    virtual ~i_video_frame(){}
    virtual const video_format_t& video_format() const = 0;
};

typedef std::unique_ptr<i_video_frame> video_frame_ptr_t;

}

}

}

#endif // I_VIDEO_FRAME_H
