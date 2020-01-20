#ifndef VIDEO_FILTER_CUSTOM_H
#define VIDEO_FILTER_CUSTOM_H

#include "video_filter.h"
#include "media/common/i_media_frame_converter.h"

namespace core
{

namespace media
{

namespace video
{

namespace filters
{

class video_filter_custom : virtual public video_filter
{
    std::unique_ptr<i_media_frame_converter>    m_input_converter;
    std::unique_ptr<i_media_frame_converter>    m_output_converter;
public:

    // video_filter interface
protected:
    bool internal_filter(i_video_frame &video_frame) const override;
};

}

}

}

}


#endif // VIDEO_FILTER_CUSTOM_H
