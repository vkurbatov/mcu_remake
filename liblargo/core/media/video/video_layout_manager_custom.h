#ifndef VIDEO_LAYOUT_MANAGER_CUSTOM_H
#define VIDEO_LAYOUT_MANAGER_CUSTOM_H

#include "i_video_layout_manager.h"
namespace core
{

namespace media
{

namespace video
{

typedef std::vector<relative_layout_list_t> layouts_list_t;

class video_layout_manager_custom : virtual public i_video_layout_manager
{
   layouts_list_t           m_custom_layouts;
public:
    video_layout_manager_custom(const layouts_list_t& custom_layouts);

    void set_layouts(const layouts_list_t& custom_layouts);
    const layouts_list_t& layouts() const;

    // i_video_layout_manager interface
public:
    bool fetch_layout(layout_id_t layout_id
                      , stream_order_t order
                      , relative_frame_rect_t &layout) override;
};

}

}

}

#endif // VIDEO_LAYOUT_MANAGER_PRESENTER_H
