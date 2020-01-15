#ifndef VIDEO_FRAME_CONVERTER_H
#define VIDEO_FRAME_CONVERTER_H

#include "media/common/i_media_frame_converter.h"
#include "media/common/ffmpeg/libav_converter.h"
#include "video_frame_rect.h"

namespace core
{

namespace media
{

namespace video
{

typedef ffmpeg::scaling_method_t scaling_method_t;

enum class aspect_ratio_mode_t
{
    scale,
    crop,
    fill
};

class video_frame_converter : virtual public i_media_frame_converter
{
    ffmpeg::libav_converter     m_ffmpeg_converter;
    aspect_ratio_mode_t         m_aspect_ratio_mode;

    frame_rect_t                m_input_area;
    frame_rect_t                m_output_area;

public:
    video_frame_converter(scaling_method_t scaling_method = scaling_method_t::default_method
                          , aspect_ratio_mode_t aspect_ratio_mode = aspect_ratio_mode_t::scale
                          , const frame_rect_t& input_area = frame_rect_t()
                          , const frame_rect_t& output_area = frame_rect_t());

    const frame_rect_t& input_area() const;
    const frame_rect_t& output_area() const;
    scaling_method_t scaling_method() const;
    aspect_ratio_mode_t aspect_ratio_mode() const;

    void set_input_area(const frame_rect_t& input_area);
    void set_output_area(const frame_rect_t& output_area);
    void set_scaling_method(scaling_method_t scaling_method);
    void set_aspect_ratio_mode(aspect_ratio_mode_t aspect_ratio_mode);

    void reset();

    // i_media_frame_converter interface
public:
    bool convert(const i_media_frame &input_frame
                 , i_media_frame &output_frame) override;

    media_frame_ptr_t convert(const i_media_frame &input_frame
                              , media_format_t &output_format) override;
};

}

}

}

#endif // VIDEO_FRAME_CONVERTER_H
