#ifndef SIMPLE_MEDIA_SINK_H
#define SIMPLE_MEDIA_SINK_H

#include "i_media_sink.h"

#include <functional>

namespace core
{

namespace media
{

typedef std::function<bool(media_frame_ptr_t frame)> simple_sink_handler_t;

class simple_media_sink : virtual public i_media_sink
{
    simple_sink_handler_t   m_sink_handler;

    static media_sink_ptr_t create(simple_sink_handler_t sink_handler = nullptr);

public:
    simple_media_sink(simple_sink_handler_t sink_handler = nullptr);

    void set_handler(simple_sink_handler_t sink_handler);

    // i_media_sink interface
public:
    bool on_frame(media_frame_ptr_t frame) override;
};

}

}

#endif // SIMPLE_MEDIA_SINK_H
