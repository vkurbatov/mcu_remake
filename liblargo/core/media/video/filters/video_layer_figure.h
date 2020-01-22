#ifndef VIDEO_LAYER_FIGURE_H
#define VIDEO_LAYER_FIGURE_H

#include "i_video_layer.h"

namespace core
{

namespace media
{

namespace video
{

namespace filters
{

enum class figure_type_t
{
    polygon,
    rectangle,
    ellipse
};

struct figure_format_t
{
    figure_type_t   figure_type;
    std::uint32_t   line_color;
    std::uint32_t   background_color;
    std::int32_t    line_width;

    figure_format_t(figure_type_t figure_type
                    , std::uint32_t line_color = 0x000000FF
                    , std::uint32_t background_color = 0
                    , std::int32_t line_width = 1);
};

typedef std::vector<frame_point_t> polyline_list_t;

class video_layer_figure : virtual public i_video_layer
{
    figure_format_t     m_figure_format;
    polyline_list_t     m_polylines;
public:
    video_layer_figure(const figure_format_t& figure_format
                     , const polyline_list_t& polylines);

    void set_format(const figure_format_t& figure_format);
    const figure_format_t& get_format() const;

    void set_polylines(const polyline_list_t& polylines);
    const polyline_list_t& get_polylines() const;

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


#endif // VIDEO_LAYER_FIGURE_H
