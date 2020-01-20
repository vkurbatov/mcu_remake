#include "magick_base.h"
#include <cstring>

#include <Magick++.h>

#define BT(val, n) ((val) >> n * 8) & 0xFF

#define RGB(color) BT(color, 3), BT(color, 2), BT(color, 1)
#define RGBA(color) RGB(color), BT(color, 0)

namespace magick
{

struct img_magick_init
{
    img_magick_init()
    {
        Magick::InitializeMagick("");
    }
} magick_init;

void draw_text(const std::string &text
               , int32_t x
               , int32_t y
               , uint32_t color
               , void *frame_data
               , int32_t width
               , int32_t height)
{
    auto frame_size = width * height * 3;

    Magick::Blob blop(frame_data, frame_size);

    //Magick::Image img()

    Magick::Image img(blop
                      , Magick::Geometry(width, height)
                      , 8
                      , "RGB");


    Magick::TypeMetric fm;
    img.fontPointsize(20);
    img.fontTypeMetrics(text, &fm);

    std::list<Magick::Drawable> draw_list;

    // draw_list.push_back(Magick::DrawableFillColor(Magick::Color(RGB(color))));
    draw_list.push_back(Magick::DrawableFillColor("white"));
    draw_list.push_back(Magick::DrawableStrokeWidth(3));
    draw_list.push_back(Magick::DrawableText(x, y, text));

    img.draw(draw_list);
    img.write(&blop);

    std::memcpy(frame_data, blop.data(), blop.length());


    //img.writePixels(Magick::QuantumType::RGBQuantum, static_cast<std::uint8_t*>(frame_data));

}

}
