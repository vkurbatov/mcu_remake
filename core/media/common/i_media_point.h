#ifndef I_MEDIA_POINT_H
#define I_MEDIA_POINT_H

#include "media/common/media_types.h"

namespace core
{

namespace media
{

class IMediaReadStatus
{
public:
	virtual ~IMediaReadStatus() = default;
	virtual bool CanRead() const = 0;
};

class IMediaReader : public IMediaReadStatus
{
public:
	virtual ~IMediaReader() = default;
	virtual std::int32_t Read(void* data, std::size_t size, std::uint32_t options = 0) = 0;
};

class IMediaWriteStatus
{
public:
	virtual ~IMediaWriteStatus() = default;
	virtual bool CanWrite() const = 0;
};

class IMediaWriter : public IMediaWriteStatus
{
public:
	virtual ~IMediaWriter() = default;

	virtual std::int32_t Write(const void* data, std::size_t size, std::uint32_t options = 0) = 0;
};

class IMediaPoint : public IMediaReader, public IMediaWriter
{
public:
	virtual ~IMediaPoint() override = default;
};

} // media

} // core

#endif // I_MEDIA_POINT_H
