#ifndef I_MEDIA_POINT_H
#define I_MEDIA_POINT_H

#include "core/media/common/media_types.h"

namespace core
{

namespace media
{

class IMediaReadStatus
{
public:
	virtual ~IMediaReadStatus() {}
	virtual bool CanRead() const = 0;
};

class IMediaReader : virtual public IMediaReadStatus
{
public:
	virtual ~IMediaReader() {}
	virtual std::int32_t Read(void* data, std::size_t size, std::uint32_t options = 0) = 0;
};

class IMediaWriteStatus
{
public:
	virtual ~IMediaWriteStatus() {}
	virtual bool CanWrite() const = 0;
};

class IMediaWriter : virtual public IMediaWriteStatus
{
public:
	virtual ~IMediaWriter() {}

	virtual std::int32_t Write(const void* data, std::size_t size, std::uint32_t options = 0) = 0;
};

class IMediaPoint : virtual public IMediaReader, virtual public IMediaWriter
{
public:
	virtual ~IMediaPoint() override {}
};

} // media

} // core

#endif // I_MEDIA_POINT_H
