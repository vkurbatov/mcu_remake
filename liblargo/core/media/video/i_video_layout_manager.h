#ifndef I_VIDEO_LAYOUT_MANAGER_H
#define I_VIDEO_LAYOUT_MANAGER_H

//#include <cstdint>
#include "video_frame_rect.h"
#include <vector>

namespace core
{

namespace media
{

namespace video
{


typedef std::uint32_t layout_id_t;
typedef std::uint32_t stream_order_t;
typedef std::vector<relative_frame_rect_t> relative_layout_list_t;

class i_video_layout_manager
{
public:
    virtual ~i_video_layout_manager(){}
    virtual bool fetch_layout(layout_id_t layout_id
                              , stream_order_t order
                              , relative_frame_rect_t& layout) = 0;
};

}

}

}

#endif // I_VIDEO_LAYOUT_MANAGER_H
