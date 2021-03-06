#include "qt_base.h"

#include <QImage>
#include <QPainter>
#include <QColor>
#include <QString>

#define BT(val, n) ((val) >> n * 8) & 0xFF

#define RGB(color) BT(color, 3), BT(color, 2), BT(color, 1)
#define RGBA(color) RGB(color), BT(color, 0)

namespace qt
{

static Qt::Alignment align_convert(h_align_t h_align
                                   , v_align_t v_align)
{
    Qt::Alignment alignment;

    switch (h_align)
    {
        case h_align_t::left:
            alignment.setFlag(Qt::AlignLeft, true);
        break;
        case h_align_t::right:
            alignment.setFlag(Qt::AlignRight, true);
        break;
        case h_align_t::center:
            alignment.setFlag(Qt::AlignHCenter, true);
        break;
    }

    switch (v_align)
    {
        case v_align_t::top:
            alignment.setFlag(Qt::AlignTop, true);
        break;
        case v_align_t::bottom:
            alignment.setFlag(Qt::AlignBottom, true);
        break;
        case v_align_t::center:
            alignment.setFlag(Qt::AlignVCenter, true);
        break;
    }

    return alignment;
}

static void set_text_format(QPainter& painter
                            , const text_format_t& text_format)
{
    painter.setPen(QColor(RGBA(text_format.color)));
    painter.setFont(QFont(QString::fromStdString(text_format.font.name)
                          , text_format.font.size
                          , text_format.font.weight
                          , text_format.font.italic));
}

static void set_draw_format(QPainter& painter
                            , const  draw_format_t& draw_format)
{

    painter.setPen(QPen(QBrush(QColor(RGBA(draw_format.border_color)
                                      )
                               )
                        , draw_format.border_weight)
                   );


    painter.setBrush(QBrush(QColor(RGBA(draw_format.background_color)
                                        )
                                 )
                          );
}

frame_size_t font_t::text_size(const std::string &text
                               , const std::string &name
                               , int32_t size
                               , int32_t weight
                               , bool italic)
{
    QFont font(QString::fromStdString(name)
               , size
               , weight
               , italic);

    QFontMetrics fm(font);

    return { fm.width(QString::fromStdString(text))
             , fm.height() * (std::count(text.begin(), text.end(), '\n') + 1)};
}

font_t::font_t(const std::string &name
               , int32_t size
               , int32_t weight
               , bool italic)
    : name(name)
    , size(size)
    , weight(weight)
    , italic(italic)
{

}

frame_size_t font_t::text_size(const std::string &text) const
{
    return text_size(text
                     , name
                     , size
                     , weight
                     , italic);
}

text_format_t::text_format_t(font_t font
                             , color_t color)
    : font(font)
    , color(color)
{

}

void draw_text(const std::string &text
               , const text_format_t &text_format
               , const frame_point_t &text_point
               , void *frame_data
               , const frame_size_t &frame_size
               , pixel_format_t pixel_format)
{
    QImage img(static_cast<std::uint8_t*>(frame_data)
               , frame_size.width
               , frame_size.height
               , static_cast<QImage::Format>(pixel_format));

    QPainter painter(&img);

    set_text_format(painter
                    , text_format);

    painter.drawText(text_point.x
                     , text_point.y
                     , QString::fromStdString(text));
}

void draw_text(const std::string &text
               , const text_format_t &text_format
               , const frame_rect_t &text_rect
               , h_align_t h_align
               , v_align_t v_align
               , void *frame_data
               , const frame_size_t &frame_size
               , pixel_format_t pixel_format)
{
    QImage img(static_cast<std::uint8_t*>(frame_data)
               , frame_size.width
               , frame_size.height
               , static_cast<QImage::Format>(pixel_format));

    QPainter painter(&img);

    set_text_format(painter
                    , text_format);

    if (pixel_format != pixel_format_t::argb32
            && pixel_format != pixel_format_t::rgba32)
    {
        painter.setOpacity(static_cast<double>(text_format.color & 0xFF) / 255.0);
    }

    painter.drawText({
                         text_rect.offset.x
                         , text_rect.offset.y
                         , text_rect.size.width
                         , text_rect.size.height
                     }
                     , QString::fromStdString(text)
                     , align_convert(h_align
                                     , v_align));
}

draw_format_t::draw_format_t(color_t border_color
                             , int32_t border_weight
                             , color_t background_color)
    : border_color(border_color)
    , border_weight(border_weight)
    , background_color(background_color)
{

}

void draw_rect(const draw_format_t& draw_format
               , const frame_rect_t& draw_rect
               , void *frame_data
               , const frame_size_t& frame_size
               , pixel_format_t pixel_format)
{
    QImage img(static_cast<std::uint8_t*>(frame_data)
               , frame_size.width
               , frame_size.height
               , static_cast<QImage::Format>(pixel_format));

    QPainter painter(&img);

    set_draw_format(painter
                    , draw_format);

    painter.drawRect(draw_rect.offset.x
                     , draw_rect.offset.y
                     , draw_rect.size.width
                     , draw_rect.size.height);

}

void draw_ellipse(const draw_format_t &draw_format
                  , const frame_rect_t &draw_rect
                  , void *frame_data
                  , const frame_size_t &frame_size
                  , pixel_format_t pixel_format)
{
    QImage img(static_cast<std::uint8_t*>(frame_data)
               , frame_size.width
               , frame_size.height
               , static_cast<QImage::Format>(pixel_format));

    QPainter painter(&img);

    set_draw_format(painter
                    , draw_format);

    painter.drawEllipse(draw_rect.offset.x
                       , draw_rect.offset.y
                       , draw_rect.size.width
                       , draw_rect.size.height);
}

void draw_polygon(const draw_format_t &draw_format
                  , const polylines_t &polylines
                  , void *frame_data
                  , const frame_size_t &frame_size
                  , pixel_format_t pixel_format)
{
    QImage img(static_cast<std::uint8_t*>(frame_data)
               , frame_size.width
               , frame_size.height
               , static_cast<QImage::Format>(pixel_format));

    QPainter painter(&img);

    set_draw_format(painter
                    , draw_format);

    QPolygon polygon;

    std::for_each(polylines.begin()
                  , polylines.end()
                  , [&polygon](const frame_point_t& point) { polygon.append({ point.x, point.y }); } );

    painter.drawPolygon(polygon);
}

void draw_image(const void *input_frame_data
                , const frame_size_t &input_frame_size
                , const frame_rect_t &input_frame_rect
                , void *output_frame_data
                , const frame_size_t& output_frame_size
                , const frame_rect_t& output_frame_rect
                , double opacity
                , pixel_format_t pixel_format)
{
    const QImage input_img(static_cast<const std::uint8_t*>(input_frame_data)
                     , input_frame_size.width
                     , input_frame_size.height
                     , static_cast<QImage::Format>(pixel_format));

    QImage output_img(static_cast<std::uint8_t*>(output_frame_data)
                     , output_frame_size.width
                     , output_frame_size.height
                     , static_cast<QImage::Format>(pixel_format));


    QPainter output_painter(&output_img);

    output_painter.setOpacity(opacity);

    output_painter.drawImage(QRect(output_frame_rect.offset.x
                                   , output_frame_rect.offset.y
                                   , output_frame_rect.size.width
                                   , output_frame_rect.size.height)
                             , input_img
                             , QRect(input_frame_rect.offset.x
                                     , input_frame_rect.offset.y
                                     , input_frame_rect.size.width
                                     , input_frame_rect.size.height)
                             , Qt::ImageConversionFlag::DiffuseAlphaDither);


}

void draw_image(const void *input_frame_data
                , const frame_size_t &input_frame_size
                , void *output_frame_data
                , const frame_size_t &output_frame_size
                , double opacity
                , pixel_format_t pixel_format)
{
    draw_image(input_frame_data
               , input_frame_size
               , { { 0, 0 }, input_frame_size }
               , output_frame_data
               , output_frame_size
               , { { 0, 0 }, output_frame_size }
               , opacity
               , pixel_format);
}

}
