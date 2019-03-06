#include "media_point.h"

#include <cerrno>

namespace core
{

namespace media
{

int32_t MediaPoint::Write(const void* data, std::size_t size, uint32_t flags)
{
	return CanWrite() ? internal_write(data, size, flags) : -EACCES;
}

int32_t MediaPoint::Read(void* data, std::size_t size, uint32_t flags)
{
	return CanRead() ? internal_read(data, size, flags) : -EACCES;
}

/*bool MediaPoint::CanRead() const
{
	return true;
}

bool MediaPoint::CanWrite() const
{
	return true;
}*/

} // media

} // core
