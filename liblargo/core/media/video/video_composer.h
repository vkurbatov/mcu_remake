#ifndef VIDEO_COMPOSER_H
#define VIDEO_COMPOSER_H

#include "media/common/i_output_media_device.h"
#include "media/video/i_video_layout_manager.h"

namespace core
{

namespace media
{

namespace video
{

struct video_composer_context_t;

typedef std::shared_ptr<video_composer_context_t> video_composer_context_ptr_t;

class video_composer : virtual public i_output_media_device
{
    video_composer_context_ptr_t    m_video_composer_context;
public:
    video_composer(const media_format_t& output_format
                   , i_video_layout_manager& video_layout_manager
                   , i_media_sink& media_sink);

    void set_stream_weight(stream_id_t stream_id
                           , double weight);
    // i_media_device interface
public:
    bool open(const std::string &uri = {}) override;
    bool close() override;
    bool is_open() const override;
    bool is_established() const override;
    media_format_list_t streams() const override;

    // i_media_control interface
public:
    const control_parameter_list_t &controls() const override;
    bool set_control(const std::string &control_name
                     , const variant &control_value) override;
    variant get_control(const std::string &control_name
                        , const variant &default_value) const override;

    // i_media_sink interface
public:
    bool on_frame(const i_media_frame &frame) override;
};


}

}

}

#endif // VIDEO_COMPOSER_H
