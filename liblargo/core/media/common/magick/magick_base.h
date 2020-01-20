#ifndef MAGICK_BASE_H
#define MAGICK_BASE_H

#include <cstdint>
#include <string>

namespace magick
{

void draw_text(const std::string& text
               , std::int32_t x
               , std::int32_t y
               , std::uint32_t color
               , void* frame_data
               , std::int32_t width
               , std::int32_t height);
}

#endif // MAGICK_BASE_H
