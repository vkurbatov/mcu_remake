#ifndef I_VIDEO_FRAME_FACTORY_H
#define I_VIDEO_FRAME_FACTORY_H

#include "i_video_frame.h"

namespace core
{

namespace media
{

namespace video
{

class i_video_frame_factory
{
public:
    virtual ~i_video_frame_factory() {}
    virtual video_frame_ptr_t create() = 0;
};

typedef std::unique_ptr<i_video_frame_factory> video_frame_factory_ptr_t;

}

}

}

#endif // I_VIDEO_FRAME_FACTORY_H
