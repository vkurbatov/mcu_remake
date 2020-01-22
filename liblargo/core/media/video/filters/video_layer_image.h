#ifndef VIDEO_LAYER_IMAGE_H
#define VIDEO_LAYER_IMAGE_H

#include "i_video_layer.h"

namespace core
{

namespace media
{

namespace video
{

namespace filters
{

struct image_decriptor_t
{
    void            *pixel_data;
    frame_size_t    frame_size;
    double          opacity;
    image_decriptor_t(void* pixel_data
                      , const frame_size_t& frame_size
                      , double opacity = 0.0);
};

class video_layer_image : virtual public i_video_layer
{
    image_decriptor_t       m_image_decriptor;
    frame_point_t           m_position;
public:
    video_layer_image(const image_decriptor_t& image_decriptor
                     , const frame_point_t& position);

    void set_image(const image_decriptor_t& image_decriptor);
    const image_decriptor_t& get_image() const;

    void set_position(const frame_point_t& position);
    const frame_point_t& get_position() const;

    // i_video_filter_custom_item interface
public:
    bool draw_layer(void *pixel_data
                      , const frame_rect_t &target_rect) override;
    frame_rect_t rect(const frame_size_t& frame_size) const override;
};

}

}

}

}

#endif // VIDEO_LAYER_IMAGE_H
