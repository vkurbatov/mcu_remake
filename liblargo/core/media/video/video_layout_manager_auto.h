#ifndef VIDEO_LAYOUT_MANAGER_AUTO_H
#define VIDEO_LAYOUT_MANAGER_AUTO_H

#include "i_video_layout_manager.h"
#include <map>

namespace core
{

namespace media
{

namespace video
{

class video_layout_manager_auto : virtual public i_video_layout_manager
{

typedef std::map<layout_id_t, relative_layout_list_t> layout_map_t;

    layout_map_t            m_layouts;

public:
    video_layout_manager_auto();
    // i_video_layout_manager interface
public:
    bool fetch_layout(layout_id_t layout_id
                      , stream_order_t order
                      , relative_frame_rect_t &layout) override;
};

}

}

}

#endif // VIDEO_LAYOUT_MANAGER_AUTO_H
