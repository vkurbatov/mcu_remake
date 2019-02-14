#ifndef MEDIA_POINT_H
#define MEDIA_POINT_H

#include "media_types.h"

namespace core
{

namespace media
{

class MediaPoint
{
public:
    virtual ~MediaPoint() = default;

    virtual std::int32_t Write(const void* data, std::size_t size, std::uint32_t flags = 0) = 0;
    virtual std::int32_t Read(void* data, std::size_t size, std::uint32_t flags = 0) = 0;

	virtual std::int32_t Write(const media_buffer_t& data, std::uint32_t flags = 0);
	virtual std::int32_t Read(media_buffer_t& data, std::uint32_t flags = 0);

	virtual bool CanRead() const;
	virtual bool CanWrite() const;
};

} // media

} // core

#endif // IO_POINT_H
