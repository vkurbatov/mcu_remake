#ifndef VIDEO_FILTER_OVERLAY_H
#define VIDEO_FILTER_OVERLAY_H

#include "video_filter.h"
#include "media/common/i_media_frame_converter.h"
#include "media/video/video_frame_converter.h"
#include "i_video_layer.h"
#include <vector>

namespace core
{

namespace media
{

namespace video
{

namespace filters
{

class video_filter_overlay : virtual public video_filter
{
    layer_list_t                                m_layer_list;
    std::unique_ptr<video_frame_converter>      m_input_converter;
    std::unique_ptr<video_frame_converter>      m_output_converter;
    mutable media_buffer_ptr_t                  m_media_buffer_ptr;

public:
    video_filter_overlay(const layer_list_t& layer_list = {});

    void set_overlays(const layer_list_t& layer_list);
    const layer_list_t& overlays() const;

protected:
    bool internal_filter(i_video_frame &video_frame) const override;
};

}

}

}

}


#endif // VIDEO_FILTER_CUSTOM_H
