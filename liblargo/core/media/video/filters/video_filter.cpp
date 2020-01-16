#include "video_filter.h"
#include "media/video/video_format.h"
#include "media/common/ffmpeg/libav_base.h"
#include "media/common/utils/format_converter.h"
#include "media/video/i_video_frame.h"

namespace core
{

namespace media
{

namespace video
{

namespace filters
{

video_filter::video_filter()
{

}

bool video_filter::filter(i_media_frame& media_frame) const
{
    return media_frame.media_format().media_type == media_type_t::video
            && !media_frame.media_format().is_encoded()
            ? internal_filter(reinterpret_cast<i_video_frame&>(media_frame))
            : false;
}

}

}

}

}
