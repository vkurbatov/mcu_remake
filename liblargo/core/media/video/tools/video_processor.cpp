#include "video_processor.h"

namespace core
{

namespace media
{

namespace video
{

namespace tools
{

struct video_processor_context_t
{
    video_processor_config_t    m_config;

    video_processor_context_t(const video_processor_config_t &config)
        : m_config(config)
    {

    }

    bool push_frame(media_frame_ptr_t frame)
    {
        return false;
    }

    media_frame_ptr_t fetch_frame(stream_id_t stream_id)
    {
        return nullptr;
    }
};

video_processor::video_processor(const video_processor_config_t &config)
    : m_video_processor_context(new video_processor_context_t(config))
{

}

bool video_processor::on_frame(media_frame_ptr_t frame)
{
    return m_video_processor_context->push_frame(frame);
}

media_frame_ptr_t video_processor::fetch_frame(stream_id_t stream_id)
{
    return m_video_processor_context->fetch_frame(stream_id);
}

}

}

}

}
