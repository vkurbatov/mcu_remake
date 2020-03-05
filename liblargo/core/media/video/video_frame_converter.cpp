#include "video_frame_converter.h"
#include "media/common/utils/format_converter.h"
#include "media/common/media_buffer.h"
#include "video_frame.h"
#include "video_utils.h"

namespace core
{

namespace media
{

namespace video
{

namespace utils
{
    ffmpeg::fragment_info_t create_fragment(const frame_rect_t& rect
                                            , const video_info_t& format)
    {
        ffmpeg::fragment_info_t fragment(rect.point.x
                                         , rect.point.y
                                         , rect.size.width
                                         , rect.size.height
                                         , format.size.width
                                         , format.size.height
                                         , media::utils::format_conversion::to_ffmpeg_video_format(format.pixel_format));

        if (fragment.frame_rect.is_null())
        {
            fragment.frame_rect.size = fragment.frame_size;
        }

        return fragment;
    }

    void fill_slices(const media_plane_list_t& plane_list
                     , void* slices[])
    {
        for (auto i = 0; i < plane_list.size(); i++)
        {
            slices[i] = plane_list[i]->data();
        }
    }

    void aspect_ratio(ffmpeg::frame_rect_t& input_rect
                      , ffmpeg::frame_rect_t& output_rec
                      , aspect_ratio_mode_t aspect_ratio_mode)
    {
        switch (aspect_ratio_mode)
        {
            case aspect_ratio_mode_t::crop:
                ffmpeg::frame_rect_t::aspect_ratio(output_rec, input_rect);
            break;
            case aspect_ratio_mode_t::fill:
                ffmpeg::frame_rect_t::aspect_ratio(input_rect, output_rec);
            break;
        }
    }

}

video_frame_converter::video_frame_converter(scaling_method_t scaling_method
                                             , aspect_ratio_mode_t aspect_ratio_mode
                                             , const frame_rect_t &input_area
                                             , const frame_rect_t &output_area)
    : m_ffmpeg_converter(scaling_method)
    , m_aspect_ratio_mode(aspect_ratio_mode)
    , m_input_area(input_area)
    , m_output_area(output_area)
{

}

const frame_rect_t& video_frame_converter::input_area() const
{
    return m_input_area;
}

const frame_rect_t& video_frame_converter::output_area() const
{
    return m_output_area;
}

scaling_method_t video_frame_converter::scaling_method() const
{
    return m_ffmpeg_converter.scaling_method();
}

aspect_ratio_mode_t video_frame_converter::aspect_ratio_mode() const
{
    return m_aspect_ratio_mode;
}

void video_frame_converter::set_input_area(const frame_rect_t &input_area)
{
    m_input_area = input_area;
}

void video_frame_converter::set_output_area(const frame_rect_t &output_area)
{
    m_output_area = output_area;
}

void video_frame_converter::set_scaling_method(scaling_method_t scaling_method)
{
    if (m_ffmpeg_converter.scaling_method() != scaling_method)
    {
        m_ffmpeg_converter.reset(scaling_method);
    }
}

void video_frame_converter::set_aspect_ratio_mode(aspect_ratio_mode_t aspect_ratio_mode)
{
    m_aspect_ratio_mode = aspect_ratio_mode;
}

void video_frame_converter::reset()
{
    m_ffmpeg_converter.reset();
}

bool video_frame_converter::convert(const i_media_frame &input_frame
                                    , i_media_frame &output_frame)
{
    const auto& input_format = input_frame.media_format(); //static_cast<const video_format_t&>(input_frame.media_format().video_info());
    const auto& output_format = output_frame.media_format(); //static_cast<const video_format_t&>(output_frame.media_format());

    if (input_format.media_type == media_type_t::video
            && output_format.media_type == media_type_t::video
            && input_format.is_convertable()
            && output_format.is_convertable())
    {
        ffmpeg::fragment_info_t input_fragment = utils::create_fragment(m_input_area
                                                         , input_format.video_info());

        ffmpeg::fragment_info_t output_fragment = utils::create_fragment(m_output_area
                                                         , output_format.video_info());


        void* input_slices[ffmpeg::max_planes] = {};
        void* output_slices[ffmpeg::max_planes] = {};

        utils::fill_slices(input_frame.planes(), input_slices);
        utils::fill_slices(output_frame.planes(), output_slices);


        auto store_output_rect = output_fragment.frame_rect;

        utils::aspect_ratio(input_fragment.frame_rect
                            , output_fragment.frame_rect
                            , m_aspect_ratio_mode);

        if (store_output_rect != output_fragment.frame_rect)
        {
            // output_frame.clear();
        }


        return m_ffmpeg_converter.convert_slices(input_fragment
                                                 , input_slices
                                                 , output_fragment
                                                 , output_slices
                                                 ) > 0;
    }

    return false;
}

media_frame_ptr_t video_frame_converter::convert(const i_media_frame &input_frame
                                                 , media_format_t &output_format)
{
    if (input_frame.media_format().media_type == media_type_t::video
            && output_format.media_type == media_type_t::video
            && input_frame.media_format().is_convertable()
            && output_format.is_convertable())
    {
        auto frame = video_frame::create(output_format
                                         , media_buffer::create(nullptr
                                                                , output_format.plane_sizes()));


        if (frame != nullptr
                && convert(input_frame
                           , *frame))
        {
            return frame;
        }
    }

    return nullptr;
}

}

}

}
