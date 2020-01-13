#ifndef I_VIDEO_FRAME_BUILDER_H
#define I_VIDEO_FRAME_BUILDER_H

#include "i_video_frame.h"

namespace core
{

namespace media
{

namespace video
{

class i_video_frame_builder
{
public:
    virtual ~i_video_frame_builder() {}
    virtual video_frame_ptr_t build() = 0;
};

}

}

}

#endif // I_VIDEO_FRAME_BUILDER_H
