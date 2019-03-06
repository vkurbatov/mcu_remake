#ifndef MEDIA_POINT_H
#define MEDIA_POINT_H

#include "media/common/i_media_point.h"

namespace core
{

namespace media
{

class MediaPoint : public IMediaPoint
{
public:
	virtual ~MediaPoint() override = default;

	virtual std::int32_t Write(const void* data, std::size_t size, std::uint32_t options = 0) override;
	virtual std::int32_t Read(void* data, std::size_t size, std::uint32_t options = 0) override;

/*	virtual bool CanRead() const override;
	virtual bool CanWrite() const override;*/

protected:

	virtual std::int32_t internal_write(const void* data, std::size_t size, std::uint32_t options = 0) = 0;
	virtual std::int32_t internal_read(void* data, std::size_t size, std::uint32_t options = 0) = 0;
};

} // media

} // core

#endif // IO_POINT_H
