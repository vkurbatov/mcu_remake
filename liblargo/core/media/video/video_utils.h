#ifndef VIDEO_UTILS_H
#define VIDEO_UTILS_H

#include "media/common/i_media_frame.h"

namespace core
{

namespace media
{

namespace video
{

namespace video_utils
{

    bool fill_slices(const i_media_frame& media_frame
                     , void* slices[]);
    bool blackout(const i_media_frame& media_frame);
    bool vertical_flip(const i_media_frame& media_frame);
    bool horizontal_flip(const i_media_frame& media_frame);
    bool fast_rotate(const i_media_frame& media_frame);

}

}

}

}

#endif // VIDEO_UTILS_H
