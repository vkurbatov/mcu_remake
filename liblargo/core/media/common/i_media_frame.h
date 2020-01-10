#ifndef I_MEDIA_FRAME_H
#define I_MEDIA_FRAME_H

#include "media_format.h"

namespace core
{

namespace media
{

class i_media_frame
{  
public:
    virtual ~i_media_frame() {}
    virtual const media_format_t& media_format() const = 0;

};

class i_media_frame_writer : virtual public i_media_frame
{
public:
    virtual ~i_media_frame_writer() {}
};

}

}

#endif // I_MEDIA_FRAME_H
