#ifndef I_MEDIA_FRAME_TRANSCODER_H
#define I_MEDIA_FRAME_TRANSCODER_H

#include "i_media_frame.h"
#include <queue>

namespace core
{

namespace media
{

typedef std::queue<media_frame_ptr_t> media_frame_queue_t;

class i_media_frame_transcoder
{
public:
    virtual bool transcode(const i_media_frame& input_frame
                           , media_frame_queue_t& frame_queue) = 0;
};

}

}


#endif // I_MEDIA_FRAME_TRANSCODER_H
