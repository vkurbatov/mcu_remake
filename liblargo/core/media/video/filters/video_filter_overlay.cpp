#include "video_filter_overlay.h"
#include "media/video/i_video_frame.h"
#include "media/common/qt/qt_base.h"
#include "media/common/media_buffer.h"
#include "media/video/video_frame.h"
#include "media/video/video_utils.h"

namespace core
{

namespace media
{

namespace video
{

namespace filters
{

const auto width_align = 32;//ffmpeg::default_frame_align;

static frame_rect_t fetch_total_rect(const layer_list_t& overlay_list
                                     , const frame_size_t& frame_size)
{
    auto i = 0;
    frame_rect_t total_rect;

    for (const auto& overlay : overlay_list)
    {
        auto rect = overlay->rect(frame_size);

        if (i == 0)
        {
            total_rect = overlay->rect(frame_size);
        }
        else
        {
            total_rect.merge(overlay->rect(frame_size));
        }
        i++;
    }

    return total_rect.cut(frame_size);
}

video_filter_overlay::video_filter_overlay(const layer_list_t& layer_list)
    : m_layer_list(layer_list)
    , m_input_converter(new video_frame_converter(scaling_method_t::x))
    , m_output_converter(new video_frame_converter(scaling_method_t::x))
    , m_media_buffer_ptr(nullptr)
{

}

bool video_filter_overlay::internal_filter(i_video_frame &video_frame) const
{
    if (m_layer_list.empty())
    {
        return false;
    }

    const frame_size_t& frame_size = video_frame.media_format().video_info().size;

    frame_rect_t overlay_rect = fetch_total_rect(m_layer_list
                                             ,  frame_size );
    if ( video_frame.media_format().is_planar())
    {
        if (overlay_rect.size.width % width_align != 0)
        {
            overlay_rect.size.width += width_align - overlay_rect.size.width % width_align;
            if (overlay_rect.size.width + overlay_rect.point.x > frame_size.width)
            {
                overlay_rect.point.x = frame_size.width - overlay_rect.size.width;
            }
        }
    }

    frame_rect_t work_rect(overlay_rect.size);

    video_info_t work_format(pixel_format_t::rgb24
                             , work_rect.size
                             , 0);

    if (m_media_buffer_ptr == nullptr
            || m_media_buffer_ptr->size() < work_format.frame_size())
    {
        m_media_buffer_ptr = media_buffer::create(nullptr
                                                  , work_format.frame_size());
    }


    auto overlay_frame = video::video_frame::create(work_format
                                                    , m_media_buffer_ptr);

    m_input_converter->set_input_area(overlay_rect);
    m_output_converter->set_output_area(overlay_rect);

    if (m_input_converter->convert(video_frame
                                   , *overlay_frame))
    {
        for (const auto& overlay : m_layer_list)
        {
            if (overlay != nullptr)
            {
                frame_rect_t real_rect = overlay->rect(frame_size);

                real_rect.point.x -= overlay_rect.point.x;
                real_rect.point.y -= overlay_rect.point.y;
                real_rect.size = work_rect.size;

                overlay->draw_layer(overlay_frame->planes()[0]->data()
                        , real_rect);
            }
        }

        return m_output_converter->convert(*overlay_frame
                                           , video_frame);
    }

    return false;
}

}

}

}

}
