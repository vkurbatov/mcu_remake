#ifndef VIDEO_FILTER_H
#define VIDEO_FILTER_H

#include "media/common/i_media_filter.h"

namespace core
{

namespace media
{

namespace video
{

class i_video_frame;

namespace filters
{

class video_filter : virtual public i_media_filter
{

public:
    video_filter();

    // i_media_filter interface
public:
    bool filter(i_media_frame& media_frame) const override;

protected:
    bool virtual internal_filter(i_video_frame& video_frame) const = 0;
};

}

}

}

}

#endif // VIDEO_FILTER_H
