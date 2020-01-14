#include "video_frame_factory.h"
#include "video_frame_buffered.h"

namespace core
{

namespace media
{

namespace video
{

video_frame_ptr_t video_frame_factory::create_frame(const video_format_t &video_format
                                                    , media::media_buffer &&media_buffer)
{
    video_frame_ptr_t video_frame_ptr;
    if (video_format.is_valid()
            && media_frame::check_media_buffer(video_format
                                        , media_buffer))
    {
        video_frame_ptr.reset(new video_frame_buffered(video_format
                                                         , std::move(media_buffer)));
    }

    return video_frame_ptr;
}

}

}

}
