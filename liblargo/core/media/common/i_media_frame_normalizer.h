#ifndef I_MEDIA_FRAME_NORMALIZER_H
#define I_MEDIA_FRAME_NORMALIZER_H

#include "i_media_frame.h"

namespace core
{

namespace media
{

class i_media_frame_normalizer
{
public:
    virtual ~i_media_frame_normalizer(){}
    virtual media_frame_ptr_t normalize(media_frame_ptr_t input_frame) = 0;
    virtual void reset() = 0;
};

}

}

#endif // I_MEDIA_FRAME_NORMALIZER_H
