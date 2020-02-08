#ifndef I_MEDIA_SINK_H
#define I_MEDIA_SINK_H

#include <cstdint>
#include <memory>

namespace core
{

namespace media
{

class i_media_frame;

class i_media_sink
{
public:
    virtual ~i_media_sink(){}

    virtual bool on_frame(const i_media_frame& frame) = 0;
};

typedef std::shared_ptr<i_media_sink> media_sink_ptr_t;

}

}

#endif // I_MEDIA_SINK_H
