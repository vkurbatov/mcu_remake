#ifndef I_MEDIA_SOURCE_H
#define I_MEDIA_SOURCE_H

#include "i_media_sink.h"

namespace core
{

namespace media
{

typedef std::uint32_t sink_id_t;

class i_media_source
{
public:
    virtual ~i_media_source(){}

    virtual sink_id_t add_sink(media_sink_ptr_t sink) = 0;
    virtual bool remove_sink(sink_id_t sink_id) = 0;
    virtual bool has_sink(sink_id_t sink_id) const = 0;
};

}

}

#endif // I_MEDIA_SOURCE_H
