#ifndef I_MEDIA_SOURCE_H
#define I_MEDIA_SOURCE_H

#include "i_media_frame.h"

namespace core
{

namespace media
{

typedef std::uint32_t sink_id_t;

class i_media_source
{
public:
    virtual ~i_media_source(){}
    virtual media_frame_ptr_t fetch_frame(stream_id_t stream_id) = 0;
};

}

}

#endif // I_MEDIA_SOURCE_H
