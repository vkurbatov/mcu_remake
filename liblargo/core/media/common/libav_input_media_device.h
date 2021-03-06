#ifndef LIBAV_INPUT_MEDIA_DEVICE_H
#define LIBAV_INPUT_MEDIA_DEVICE_H

#include "i_media_sink.h"
#include "i_input_media_device.h"

#include "media/common/ffmpeg/libav_stream_capturer.h"

namespace core
{

namespace media
{


class libav_input_media_device : virtual public i_input_media_device
{
    std::uint32_t                       m_video_frame_counter;
    std::uint32_t                       m_audio_frame_counter;
    ffmpeg::libav_stream_capturer       m_libav_stream_capturer;

public:
    libav_input_media_device(i_media_sink& media_sink);

    // i_media_device interface
public:
    bool open(const std::string &uri) override;
    bool close() override;
    bool is_open() const override;
    bool is_established() const override;

    media_format_list_t streams() const override;

    // i_media_control interface
public:
    const control_parameter_list_t& controls() const override;
    bool set_control(const std::string &control_name
                     , const variant& control_value) override;
    variant get_control(const std::string &control_name
                        , const variant& default_value) const override;
};

}

}

#endif // LIBAV_MEDIA_DEVICE_H
