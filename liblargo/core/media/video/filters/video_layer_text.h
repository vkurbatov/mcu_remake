#ifndef video_layer_TEXT_H
#define video_layer_TEXT_H

#include "i_video_layer.h"

namespace core
{

namespace media
{

namespace video
{

namespace filters
{

struct text_format_t
{
    std::string     font;
    std::uint32_t   color;
    std::int32_t    height;
    std::int32_t    weight;
    bool            italic;

    text_format_t(std::string font = "Times"
                  , std::uint32_t color = 0x00000000
                  , std::int32_t height = 12
                  , std::int32_t weight = 10
                  , bool italic = false);

    frame_size_t text_size(const std::string& text) const;
};

class video_layer_text : virtual public i_video_layer
{    
    std::string             m_text;
    text_format_t           m_text_format;
    frame_point_t           m_position;
public:
    video_layer_text(const std::string& text
                     , const text_format_t& text_format
                     , const frame_point_t& position);

    void set_text(const std::string& text);
    const std::string& get_text() const;

    void set_format(const text_format_t& text_format);
    const text_format_t& get_format() const;

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

#endif // VIDEO_DRAWING_TEXT_FILTER_H
