#ifndef MEDIA_FRAME_H
#define MEDIA_FRAME_H

#include "i_media_frame.h"

namespace core
{

namespace media
{

class media_frame : virtual public i_media_frame
{


public:
    virtual ~media_frame() override {}

    // i_media_frame interface
public:
    const media_format_t &media_format() const override;

};

}

}

#endif // MEDIA_FRAME_H
