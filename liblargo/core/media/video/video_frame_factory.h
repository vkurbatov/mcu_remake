#ifndef VIDEO_FRAME_FACTORY_H
#define VIDEO_FRAME_FACTORY_H

#include "i_video_frame_factory.h"
#include "media/common/media_buffer.h"

namespace core
{

namespace media
{

namespace video
{

class video_frame_factory : virtual public i_video_frame_factory
{
    // i_video_frame_factory interface
public:
    static video_frame_ptr_t create_frame(const video_format_t& video_format
                                          , media::media_buffer&& media_buffer);
public:
    video_frame_ptr_t create() override;
};

}

}

}

#endif // VIDEO_FRAME_FACTORY_H
