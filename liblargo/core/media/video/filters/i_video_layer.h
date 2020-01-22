#ifndef I_VIDEO_LAYER_H
#define I_VIDEO_LAYER_H

#include "media/common/i_media_filter.h"
#include "media/video/video_frame_rect.h"

namespace core
{

namespace media
{

namespace video
{

//class frame_size_t;

namespace filters
{

class i_video_layer
{
public:
    virtual ~i_video_layer(){}
    virtual bool draw_layer(void* pixel_data
                            , const frame_rect_t& target_rect) = 0;
    virtual frame_rect_t rect(const frame_size_t& frame_size) const = 0;
};

typedef std::shared_ptr<i_video_layer> video_layer_ptr_t;
typedef std::vector<video_layer_ptr_t> layer_list_t;

}

}

}

}

#endif // I_VIDEO_FILTER_CUSTOM_ITEM_H
