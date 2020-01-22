#ifndef I_MEDIA_FILTER_H
#define I_MEDIA_FILTER_H

#include "i_media_frame.h"
#include <memory>

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

typedef std::shared_ptr<i_media_filter> media_filter_ptr_t;

}

}

#endif // I_MEDIA_FILTER_H
