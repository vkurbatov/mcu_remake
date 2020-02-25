#include "device_info.h"
#include <vector>

namespace core
{

namespace media
{

template<typename T>
static T operator & (const T& value, const T& flag)
{
    return static_cast<T>(static_cast<std::uint32_t>(value) & static_cast<std::uint32_t>(flag));
}


device_type_t device_info_t::device_type_from_uri(const std::string &uri)
{
    static const std::vector<std::vector<std::string>> prefixes =
    {
        { "" }
        , { "v4l2://", "camera://", "/dev/video"}
        , { "/", "file://" }
        , { "rtsp://" }
        , { "rtmp://" }
        , { "vnc://" }
    };

    auto i = 1;

    for (const auto& pa : prefixes)
    {
        for (const auto& p : pa)
        {
            if (uri.find(p) == 0)
            {
                return static_cast<device_type_t>(i);
            }
        }
        i++;
    }

    return device_type_t::undefined;
}

device_info_t::device_info_t(device_class_t device_class
                            , device_direction_t device_direction
                            , std::string name
                            , std::string description
                            , std::string uri
                            , std::uint32_t device_id)
    : device_class(device_class)
    , device_direction(device_direction)
    , name(name)
    , description(description)
    , uri(uri)
    , device_id(device_id)
{

}

bool device_info_t::is_audio_device() const
{
    return (device_class & device_class_t::audio) != device_class_t::undefined;
}

bool device_info_t::is_video_device() const
{
    return (device_class & device_class_t::video) != device_class_t::undefined;
}

bool device_info_t::is_input_device() const
{
    return (device_direction & device_direction_t::input) != device_direction_t::undefined;
}

bool device_info_t::is_output_device() const
{
    return (device_direction & device_direction_t::output) != device_direction_t::undefined;
}

device_type_t device_info_t::type() const
{
    return device_type_from_uri(uri);
}

}

}
