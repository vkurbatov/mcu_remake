#ifndef MEDIA_DEVICE_MANAGER_H
#define MEDIA_DEVICE_MANAGER_H

#include "i_media_device_manager.h"
#include "media/video/video_format.h"
#include "media/audio/audio_format.h"

namespace core
{

namespace media
{

const video::video_info_t rtmp_default_video_format = { video::pixel_format_t::h264
                                                        , { 1280, 720 }
                                                        , { 30 }
                                                      };

const audio::audio_info_t rtmp_default_audio_format = { audio::sample_format_t::aac
                                                        , 32000
                                                        , 1
                                                      };

struct manager_config_t
{
    std::uint32_t           v4l2_buffers_count;
    std::uint32_t           vnc_fps;

    video::video_info_t     rtmp_video_format;
    audio::audio_info_t     rtmp_audio_format;
    std::uint32_t           rtmp_video_bitrate;
    std::uint32_t           rtmp_video_gop;
    std::uint32_t           rtmp_audio_bitrate;
    std::uint32_t           rtmp_audio_frame_size;

    std::string             media_files_directory;

    manager_config_t(std::uint32_t v4l2_buffers_count = 2
                    , std::uint32_t vnc_fps = 25
                    , const video::video_info_t& rtmp_video_format = rtmp_default_video_format
                    , const audio::audio_info_t& rtmp_audio_format = rtmp_default_audio_format
                    , std::uint32_t rtmp_video_bitrate = 1000000
                    , std::uint32_t rtmp_video_gop = 12
                    , std::uint32_t rtmp_audio_bitrate = 128000
                    , std::uint32_t rtmp_audio_frame_size = 1024
            , const std::string& media_files_directory = "/home/user");
};

typedef std::map<device_id_t,device_info_t> device_info_map_t;

class media_device_manager : virtual public i_media_device_manager
{
    device_info_map_t       m_custom_device_list;
    manager_config_t        m_config;
    device_id_t             m_device_id;

public:
    media_device_manager(const manager_config_t& config={});

    // i_media_device_manager interface
public:
    device_info_list_t device_info_list(device_class_t device_class
                                        , device_direction_t device_direction) const override;
    device_id_t register_device(const device_info_t &device_info) override;
    bool unregister_device(device_id_t device_id) override;

    input_media_managed_device_ptr_t create_input_device(const device_info_t &device_info
                                                         , media_sink_ptr_t sink) override;
    output_media_managed_device_ptr_t create_output_device(const device_info_t &device_info) override;
};

}

}

#endif // MEDIA_DEVICE_MANAGER_H
