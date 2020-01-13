#ifndef VIDEO_FRAME_FACTORY_H
#define VIDEO_FRAME_FACTORY_H

#include "i_video_frame_factory.h"

namespace core
{

namespace media
{

namespace video
{

class video_frame_factory : virtual public i_video_frame_factory
{
    // i_video_frame_factory interface
    static video_frame_ptr_t create_frame(const video_format_t& video_format
                                          , const void *data = nullptr
                                          , std::size_t size = 0);
public:
    video_frame_ptr_t create() override;
};

}

}

}

#endif // VIDEO_FRAME_FACTORY_H
