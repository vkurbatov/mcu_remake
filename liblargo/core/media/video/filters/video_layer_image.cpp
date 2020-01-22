#include "video_layer_image.h"
#include "media/common/qt/qt_base.h"

namespace core
{

namespace media
{

namespace video
{

namespace filters
{

image_decriptor_t::image_decriptor_t(void *pixel_data
                                     , const frame_size_t &frame_size
                                     , double opacity)
    : pixel_data(pixel_data)
    , frame_size(frame_size)
    , opacity(opacity)
{

}

//-----------------------------------------------------------------------------------
video_layer_image::video_layer_image(const image_decriptor_t &image_decriptor
                                     , const frame_point_t &position)
    : m_image_decriptor(image_decriptor)
    , m_position(position)
{

}

void video_layer_image::set_image(const image_decriptor_t &image_decriptor)
{
    m_image_decriptor = image_decriptor;
}

const image_decriptor_t &video_layer_image::get_image() const
{
    return m_image_decriptor;
}

void video_layer_image::set_position(const frame_point_t &position)
{
    m_position = position;
}

const frame_point_t &video_layer_image::get_position() const
{
    return m_position;
}

bool video_layer_image::draw_layer(void *pixel_data
                                   , const frame_rect_t &target_rect)
{
    if (m_image_decriptor.pixel_data != nullptr)
    {
        qt::draw_image(m_image_decriptor.pixel_data
                       , { m_image_decriptor.frame_size.width
                           , m_image_decriptor.frame_size.height }
                       , { 0
                           , 0
                           , m_image_decriptor.frame_size.width
                           , m_image_decriptor.frame_size.height }
                       , pixel_data
                       , { target_rect.size.width
                           , target_rect.size.height }
                       ,  { target_rect.point.x
                            , target_rect.point.y
                            , m_image_decriptor.frame_size.width
                            , m_image_decriptor.frame_size.height }
                       , m_image_decriptor.opacity
                       );
        return true;
    }

    return false;
}

frame_rect_t video_layer_image::rect(const frame_size_t &frame_size) const
{
    return { m_position, m_image_decriptor.frame_size };
}

}

}

}

}
