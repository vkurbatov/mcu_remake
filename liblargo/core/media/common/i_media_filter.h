#ifndef I_MEDIA_FILTER_H
#define I_MEDIA_FILTER_H

#include "i_media_frame.h"

namespace core
{

namespace media
{

class i_media_filter
{
public:
    virtual ~i_media_filter(){}
    virtual bool filter(i_media_frame& media_frame) const = 0;
};

}

}

#endif // I_MEDIA_FILTER_H
