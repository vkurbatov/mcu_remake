#ifndef I_MEDIA_FRAME_TRANSCODER_H
#define I_MEDIA_FRAME_TRANSCODER_H

#include "i_media_frame.h"

namespace core
{

namespace media
{

class i_media_frame_transcoder
{
public:
    virtual bool transcode(const i_media_frame& input_frame
                           , i_media_frame& output_frame) = 0;

    virtual media_frame_ptr_t transcode(const i_media_frame& input_frame
                                        , media_format_t& output_format) = 0;
};

}

}


#endif // I_MEDIA_FRAME_TRANSCODER_H
