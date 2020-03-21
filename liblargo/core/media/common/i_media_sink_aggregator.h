#ifndef I_MEDIA_SINK_AGGREGATOR_H
#define I_MEDIA_SINK_AGGREGATOR_H

#include "i_media_sink.h"

namespace core
{

namespace media
{

class i_media_sink_aggregator
{
public:
    virtual ~i_media_sink_aggregator(){}

    virtual bool on_frame(media_frame_ptr_t frame) = 0;

    virtual bool add_sink(media_sink_ptr_t sink_ptr) = 0;
    virtual bool remove_sink(media_sink_ptr_t sink_ptr) = 0;
    virtual const media_sink_list_t& sinks() const = 0;
};

}

}

#endif // I_MEDIA_SINK_AGGREGATOR_H
