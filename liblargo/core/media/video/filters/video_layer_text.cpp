#include "video_layer_text.h"
#include "media/common/qt/qt_base.h"

#include <cstring>

namespace core
{

namespace media
{

namespace video
{

namespace filters
{

text_format_t::text_format_t(std::string font
                             , uint32_t color
                             , int32_t height
                             , int32_t weight
                             , bool italic)
    : font(font)
    , color(color)
    , height(height)
    , weight(weight)
    , italic(italic)
{

}

frame_size_t text_format_t::text_size(const std::string &text) const
{
    auto sz = qt::font_t::text_size(text
                                    , font
                                    , height
                                    , weight
                                    , italic);

    return { sz.width, sz.height };
}
//-----------------------------------------------------------------------------------------------------------

video_layer_text::video_layer_text(const std::string &text
                                   , const text_format_t &text_format
                                   , const frame_point_t &position)
    : m_text(text)
    , m_text_format(text_format)
    , m_position(position)
{

}

void video_layer_text::set_text(const std::string &text)
{
    m_text = text;
}

const std::string& video_layer_text::get_text() const
{
    return m_text;
}

void video_layer_text::set_format(const text_format_t &text_format)
{
    m_text_format = text_format;
}

const text_format_t& video_layer_text::get_format() const
{
    return m_text_format;
}

void video_layer_text::set_position(const frame_point_t &position)
{
    m_position = position;
}

const frame_point_t &video_layer_text::get_position() const
{
    return m_position;
}

bool video_layer_text::draw_layer(void *pixel_data
                                   , const frame_rect_t &target_rect)
{
    qt::font_t font(m_text_format.font
                    , m_text_format.height
                    , m_text_format.weight
                    , m_text_format.italic);

    qt::text_format_t format(font
                          , m_text_format.color);
    qt::draw_text(m_text
                  , format
                  , { target_rect.point.x, target_rect.point.y + font.text_size(m_text).height }
                  , pixel_data
                  , { target_rect.size.width, target_rect.size.height }
                  );

    return true;
}

frame_rect_t video_layer_text::rect(const frame_size_t &frame_size) const
{

    auto size = m_text_format.text_size(m_text);

    return { m_position.x
            , m_position.y
            , size.width
            , size.height };
}

}

}

}

}
