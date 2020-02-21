#ifndef LIBAV_OUTPUT_MEDIA_DEVICE_H
#define LIBAV_OUTPUT_MEDIA_DEVICE_H

#include "i_media_sink.h"
#include "i_media_device.h"
#include "i_media_control.h"

#include "media/common/ffmpeg/libav_stream_publisher.h"

namespace core
{

namespace media
{

class libav_output_media_device : virtual public i_media_device
        , virtual public i_media_control
        , virtual public i_media_sink
{
    ffmpeg::libav_stream_publisher      m_libav_stream_publisher;
    media_format_list_t                 m_media_streams;

public:
    libav_output_media_device(const media_format_list_t& media_streams);
    bool set_media_streams(const media_format_list_t& media_streams);

    // i_media_device interface
public:
    bool open(const std::string &uri) override;
    bool close() override;
    bool is_open() const override;
    bool is_established() const override;
    media_format_list_t streams() const override;

    // i_media_control interface
public:
    const control_parameter_list_t &controls() const override;
    bool set_control(const std::string &control_name, const variant& control_value) override;
    variant get_control(const std::string &control_name, const variant& control_value) const override;

    // i_media_sink interface
public:
    bool on_frame(const i_media_frame &frame) override;
};

}

}

#endif // LIBAV_OUTPUT_MEDIA_DEVICE_H
