#ifndef VIDEO_FRAME_PROCESSOR_H
#define VIDEO_FRAME_PROCESSOR_H

#include "media/common/i_media_control.h"
#include "media/common/i_media_sink.h"
#include "filters/video_filter_flip.h"
#include "video_frame_normalizer.h"

namespace core
{

namespace media
{

namespace video
{

typedef std::vector<media_sink_ptr_t> media_sink_list_t;

class video_frame_processor : virtual public i_media_control
        , virtual public i_media_sink
{
    video_frame_normalizer              m_normalizer;
    filters::video_filter_flip          m_flip_filter;
    media_sink_list_t                   m_sinks;
    control_parameter_list_t            m_controls;
public:
    video_frame_processor();

    bool add_sink(media_sink_ptr_t sink_ptr);
    bool remove_sink(media_sink_ptr_t sink_ptr);
    const media_sink_list_t& sinks() const;

    // i_media_sink interface
public:
    bool on_frame(media_frame_ptr_t frame) override;

    // i_media_control interface
public:
    const control_parameter_list_t &controls() const override;
    bool set_control(const std::string &control_name, const variant &control_value) override;
    variant get_control(const std::string &control_name, const variant &default_value) const override;
};

}

}

}

#endif // VIDEO_FRAME_PROCESSOR_H
