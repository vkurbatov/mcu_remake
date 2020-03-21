#ifndef I_MEDIA_SINK_H
#define I_MEDIA_SINK_H

#include "i_media_frame.h"


namespace core
{

namespace media
{


class i_media_sink
{
public:
    virtual ~i_media_sink(){}

    virtual bool on_frame(media_frame_ptr_t frame) = 0;
};

typedef std::shared_ptr<i_media_sink> media_sink_ptr_t;
typedef std::vector<media_sink_ptr_t> media_sink_list_t;

}

}

#endif // I_MEDIA_SINK_H
