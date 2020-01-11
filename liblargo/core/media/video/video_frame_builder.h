#ifndef VIDEO_FRAME_BUILDER_H
#define VIDEO_FRAME_BUILDER_H

#include "i_video_frame_factory.h"

namespace core
{

namespace media
{

namespace video
{

typedef std::unique_ptr<i_video_frame_factory> video_frame_factory_ptr_t;

namespace factory_builder
{
    video_frame_factory_ptr_t create_video_frame_factory(const video_format_t& video_format
                                                         , std::size_t size = 0);

}

}

}

}
#endif // VIDEO_FRAME_BUILDER_H
