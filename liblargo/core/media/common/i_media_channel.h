#ifndef I_MEDIA_CHANNEL_H
#define I_MEDIA_CHANNEL_H

#include <cstdint>

namespace core
{

namespace media
{

class i_media_channel
{

public:
    virtual ~i_media_channel() {}

    virtual bool open() = 0;
    virtual bool close() = 0;
    virtual bool is_open() const = 0;
    virtual bool is_established() const = 0;
};

}

}

#endif // I_MEDIA_CHANNEL_H
