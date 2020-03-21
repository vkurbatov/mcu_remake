#include "simple_media_sink.h"

namespace core
{

namespace media
{

media_sink_ptr_t simple_media_sink::create(simple_sink_handler_t sink_handler)
{
    return media_sink_ptr_t(new simple_media_sink(sink_handler));
}

simple_media_sink::simple_media_sink(simple_sink_handler_t sink_handler)
    : m_sink_handler(sink_handler)
{

}

void simple_media_sink::set_handler(simple_sink_handler_t sink_handler)
{
    m_sink_handler = sink_handler;
}

bool simple_media_sink::on_frame(media_frame_ptr_t frame)
{
    return m_sink_handler != nullptr
            && m_sink_handler(frame);
}

}

}
