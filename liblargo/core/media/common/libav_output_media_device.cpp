#include "libav_output_media_device.h"

namespace core
{

namespace media
{

libav_output_media_device::libav_output_media_device(media_format_list_t media_streams)
    : m_media_streams(media_streams)
{

}

bool libav_output_media_device::set_media_streams(media_format_list_t media_streams)
{
    if (!m_libav_stream_publisher.is_opened())
    {
       // m_media_streams = media_streams;
        return true;
    }

    return false;
}

bool libav_output_media_device::open(const std::string &uri)
{

}

bool libav_output_media_device::close()
{

}

bool libav_output_media_device::is_open() const
{

}

bool libav_output_media_device::is_established() const
{

}

media_format_list_t libav_output_media_device::streams() const
{
    return m_media_streams;
}

const control_parameter_list_t &libav_output_media_device::controls() const
{

}

bool libav_output_media_device::set_control(const std::string &control_name
                                            , const variant &control_value)
{

}

variant libav_output_media_device::get_control(const std::string &control_name
                                               , const variant &control_value) const
{

}

bool libav_output_media_device::on_frame(const i_media_frame &frame)
{

}

}

}
