#include "media_device_manager.h"
#include "libav_input_media_device.h"
#include "libav_output_media_device.h"
#include "v4l2_input_media_device.h"
#include "vnc_input_media_device.h"
#include "codec_params.h"
#include "media/common/ffmpeg/libav_base.h"
#include "media/common/base/filesystem.h"

#include <set>

namespace core
{

namespace media
{


static input_media_device_ptr_t create_input_device(const device_info_t& device_info
                                                    , i_media_sink& media_sink
                                                    , const manager_config_t& config)
{
    switch(device_info.type())
    {
        case device_type_t::camera:
            return input_media_device_ptr_t(new v4l2_input_media_device(media_sink
                                                                        , config.v4l2_buffers_count));
        break;
        case device_type_t::rtsp:
        case device_type_t::file:
        case device_type_t::rtmp:
            return input_media_device_ptr_t(new libav_input_media_device(media_sink));
        break;
        case device_type_t::vnc:
            return input_media_device_ptr_t(new vnc_input_media_device(media_sink
                                                                       , vnc::vnc_config_t(config.vnc_fps)));
        break;
    }
    return nullptr;
}

static media_format_list_t get_rtmp_format_list(const manager_config_t& config)
{
    media_format_t video_format(config.rtmp_video_format, 0);
    video_format.codec_params().bitrate = config.rtmp_video_bitrate;
    video_format.codec_params().gop_size = config.rtmp_video_gop;

    media_format_t audio_format(config.rtmp_audio_format, 1);
    audio_format.codec_params().bitrate = config.rtmp_audio_bitrate;
    audio_format.codec_params().gop_size = config.rtmp_audio_frame_size;

    return { video_format, audio_format };
}

static output_media_device_ptr_t create_output_device(const device_info_t& device_info
                                                    , const manager_config_t& config)
{
    switch(device_info.type())
    {
        case device_type_t::rtmp:
        case device_type_t::file:
            return output_media_device_ptr_t(new libav_output_media_device(get_rtmp_format_list(config)));
        break;
    }

    return nullptr;
}

class input_media_managed_device : virtual public i_input_media_managed_device
        , virtual public i_media_sink
{
    device_info_t                       m_device_info;
    media_sink_ptr_t                    m_media_sink;
    input_media_device_ptr_t            m_input_media_device;


public:
    input_media_managed_device(const device_info_t& device_info
                               , media_sink_ptr_t media_sink
                               , const manager_config_t& config)
        : m_device_info(device_info)
        , m_media_sink(media_sink)
        , m_input_media_device(create_input_device(m_device_info
                                                   , *this
                                                   , config))
    {

    }

    const input_media_device_ptr_t& input_media_device() const
    {
        return m_input_media_device;
    }

    // i_media_control interface
public:
    const control_parameter_list_t &controls() const override
    {
        return m_input_media_device->controls();
    }
    bool set_control(const std::string &control_name
                     , const variant &control_value) override
    {
        return m_input_media_device->set_control(control_name
                                                 , control_value);
    }
    variant get_control(const std::string &control_name
                        , const variant &default_value) const override
    {

        return m_input_media_device->get_control(control_name
                                                 , default_value);
    }

    // i_media_device interface
public:
    bool open(const std::string &uri) override
    {
        return m_input_media_device->open(uri.empty()
                                          ? m_device_info.uri
                                          : uri);
    }
    bool close() override
    {
        return m_input_media_device->close();
    }
    bool is_open() const override
    {
        return m_input_media_device->is_open();
    }
    bool is_established() const override
    {
        return m_input_media_device->is_established();
    }
    media_format_list_t streams() const override
    {
        return m_input_media_device->streams();
    }

    // i_media_device_info interface
public:
    const device_info_t &device_info() const override
    {
        return m_device_info;
    }

    // i_media_sink interface
public:
    bool on_frame(media_frame_ptr_t frame) override
    {
        return m_media_sink != nullptr
                && m_media_sink->on_frame(frame);
    }
};

class output_media_managed_device : public i_output_media_managed_device
{
    device_info_t                   m_device_info;
    output_media_device_ptr_t       m_output_media_device;
public:
    output_media_managed_device(const device_info_t& device_info
                                , const manager_config_t& config)
        : m_device_info(device_info)
        , m_output_media_device(create_output_device(m_device_info
                                                     , config))
    {

    }

    const output_media_device_ptr_t& output_media_device() const
    {
        return m_output_media_device;
    }

    // i_media_control interface
public:
    const control_parameter_list_t &controls() const override
    {
        return m_output_media_device->controls();
    }

    bool set_control(const std::string &control_name
                     , const variant &control_value) override
    {
        return m_output_media_device->set_control(control_name
                                                  , control_value);
    }
    variant get_control(const std::string &control_name
                        , const variant &default_value) const override
    {
        return m_output_media_device->get_control(control_name
                                                  , default_value);
    }

    // i_media_device interface
public:
    bool open(const std::string &uri) override
    {
        return m_output_media_device->open(uri.empty()
                                           ? m_device_info.uri
                                           : uri);
    }
    bool close() override
    {
        return m_output_media_device->close();
    }
    bool is_open() const override
    {
        return m_output_media_device->is_open();
    }
    bool is_established() const override
    {
        return m_output_media_device->is_established();
    }
    media_format_list_t streams() const override
    {
        return m_output_media_device->streams();
    }

    // i_media_sink interface
public:
    bool on_frame(media_frame_ptr_t frame) override
    {
        return m_output_media_device->on_frame(frame);
    }

    // i_media_device_info interface
public:
    const device_info_t &device_info() const override
    {
        return m_device_info;
    }
};
//--------------------------------------------------------------
manager_config_t::manager_config_t(uint32_t v4l2_buffers_count
                                   , std::uint32_t vnc_fps
                                   , const video::video_info_t& rtmp_video_format
                                   , const audio::audio_info_t& rtmp_audio_format
                                   , std::uint32_t rtmp_video_bitrate
                                   , std::uint32_t rtmp_video_gop
                                   , std::uint32_t rtmp_audio_bitrate
                                   , std::uint32_t rtmp_audio_frame_size
                                   , const std::string& media_files_directory)
    : v4l2_buffers_count(v4l2_buffers_count)
    , vnc_fps(vnc_fps)
    , rtmp_video_format(rtmp_video_format)
    , rtmp_audio_format(rtmp_audio_format)
    , rtmp_video_bitrate(rtmp_video_bitrate)
    , rtmp_video_gop(rtmp_video_gop)
    , rtmp_audio_bitrate(rtmp_audio_bitrate)
    , rtmp_audio_frame_size(rtmp_audio_frame_size)
    , media_files_directory(media_files_directory)
{

}
//--------------------------------------------------------------
media_device_manager::media_device_manager(const manager_config_t& config)
    : m_config(config)
    , m_device_id(0)
{

}

device_info_list_t media_device_manager::device_info_list(device_class_t device_class
                                                          , device_direction_t device_direction) const
{
    device_info_list_t device_list;

    bool is_audio = device_class == device_class_t::audio || device_class == device_class_t::audio_video;
    bool is_video = device_class == device_class_t::video || device_class == device_class_t::audio_video;
    bool is_source = device_direction == device_direction_t::input;

    if (is_audio)
    {
        auto libav_devices = ffmpeg::device_info_t::device_list(ffmpeg::media_type_t::audio
                                                                , is_source
                                                                , "pulse");

        for (const auto& d : libav_devices)
        {
            device_list.emplace_back(device_info_t(device_class_t::audio
                                                   , device_direction
                                                   , d.name
                                                   , d.description
                                                   , d.to_uri()
                                                   , hw_device_id));
        }
    }
    if (is_video)
    {
        auto libav_devices = ffmpeg::device_info_t::device_list(ffmpeg::media_type_t::video
                                                                , is_source
                                                                , "v4l2");

        for (const auto& d : libav_devices)
        {
            device_list.emplace_back(device_info_t(device_class_t::audio
                                                   , device_direction
                                                   , d.name
                                                   , d.description
                                                   , d.to_uri()
                                                   , hw_device_id));
        }
    }

    for (const auto& f : filesystem::get_files(m_config.media_files_directory))
    {
        static std::set<std::string> media_exts = { "mp4", "avi", "gif", "mpeg" };
        // auto ext = filesystem::extract_file_extension(f);

        auto it = media_exts.find(filesystem::extract_file_extension(f));

        if (it != media_exts.end())
        {

            device_list.push_back(device_info_t(device_class_t::video
                                                , device_direction
                                                , ""
                                                , ""
                                                , f
                                                , hw_device_id));
        }
    }

    for (const auto& d : m_custom_device_list)
    {
        if ((d.second.is_audio_device() == is_audio) || (d.second.is_video_device() == is_video)
                && d.second.device_direction == device_direction)
        {
            device_list.push_back(d.second);
        }
    }

    return device_list;
}

device_id_t media_device_manager::register_device(const device_info_t &device_info)
{
    device_info_t new_device = device_info;
    new_device.device_id = m_device_id++;
    m_custom_device_list.emplace(std::make_pair(new_device.device_id
                                                , new_device));

    return new_device.device_id;
}

bool media_device_manager::unregister_device(device_id_t device_id)
{
    auto it = m_custom_device_list.find(device_id);

    if (it != m_custom_device_list.end())
    {
        m_custom_device_list.erase(it);
        return true;
    }

    return false;
}

input_media_managed_device_ptr_t media_device_manager::create_input_device(const device_info_t &device_info
                                                                           , media_sink_ptr_t sink)
{
    std::shared_ptr<input_media_managed_device> input_media_device(new input_media_managed_device(device_info
                                                                                                  , sink
                                                                                                  , m_config));

    if (input_media_device->input_media_device() != nullptr)
    {
        return input_media_device;
    }

    return nullptr;

}

output_media_managed_device_ptr_t media_device_manager::create_output_device(const device_info_t &device_info)
{
    std::shared_ptr<output_media_managed_device> output_media_device(new output_media_managed_device(device_info
                                                                                                     , m_config));

    if (output_media_device->output_media_device() != nullptr)
    {
        return output_media_device;
    }

    return nullptr;
}

}

}
