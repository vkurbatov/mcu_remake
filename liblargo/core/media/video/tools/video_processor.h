#ifndef VIDEO_PROCESSOR_H
#define VIDEO_PROCESSOR_H

#include "media/common/i_media_device_manager.h"
#include "media/common/i_media_sink.h"
#include "media/common/i_media_source.h"

namespace core
{

namespace media
{

namespace video
{

namespace tools
{

struct video_processor_context_t;
typedef std::shared_ptr<video_processor_context_t> video_processor_context_ptr_t;

struct video_processor_config_t
{

};


class video_processor : virtual public i_media_source
        , virtual public i_media_sink
{
    video_processor_context_ptr_t   m_video_processor_context;

public:
    video_processor(const video_processor_config_t& config = {});

    // i_media_sink interface
public:
    bool on_frame(media_frame_ptr_t frame) override;

    // i_media_source interface
public:
    media_frame_ptr_t fetch_frame(stream_id_t stream_id) override;
};

}

}

}

}

#endif // VIDEO_PROCESSOR_H
