#ifndef DEVICE_INFO_H
#define DEVICE_INFO_H

#include <string>

namespace core
{

namespace media
{

typedef std::int32_t device_id_t;
const device_id_t hw_device_id = -1;
const device_id_t no_device_id = -1;

enum class device_class_t
{
    undefined,
    video = 1,
    audio = 2,
    audio_video = device_class_t::video | device_class_t::audio
};


enum class device_direction_t
{
    undefined,
    input = 1,
    output = 2,
 //   input_output = device_direction_t::input | device_direction_t::output
};

enum class device_type_t
{
    undefined,
    camera,
    file,
    rtsp,
    rtmp,
    vnc,
    alsa,
    pulse
};


struct device_info_t
{
    device_class_t device_class;
    device_direction_t device_direction;
    std::string name;
    std::string description;
    std::string uri;
    device_id_t device_id;

    static device_type_t device_type_from_uri(const std::string& uri);

    device_info_t(device_class_t device_class = device_class_t::undefined
                  , device_direction_t device_direction = device_direction_t::undefined
                  , std::string name = {}
                  , std::string description = {}
                  , std::string uri = {}
                  , device_id_t device_id = {});

    bool is_audio_device() const;
    bool is_video_device() const;
    bool is_input_device() const;
    bool is_output_device() const;

    device_type_t type() const;
    const std::string& to_string() const;
};

}

}

#endif // DEVICE_INFO_H
