#ifndef I_VIDEO_FRAME_FACTORY_H
#define I_VIDEO_FRAME_FACTORY_H

#include "i_video_frame.h"

#include <memory>

namespace core
{

namespace media
{

namespace video
{

typedef std::unique_ptr<i_video_frame> video_frame_ptr_t;

class i_video_frame_factory
{
public:
    virtual video_frame_ptr_t create() = 0;
};

}

}

}

#endif // I_VIDEO_FRAME_FACTORY_H
