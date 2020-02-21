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
};

typedef std::shared_ptr<i_video_frame> video_frame_ptr_t;

}

}

}

#endif // I_VIDEO_FRAME_H
