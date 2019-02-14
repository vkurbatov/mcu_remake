#include "media_point.h"

namespace core
{

namespace media
{

int32_t MediaPoint::Write(const media_buffer_t& data, uint32_t flags)
{
	return Write(data.data(), data.size(), flags);
}

int32_t MediaPoint::Read(media_buffer_t& data, uint32_t flags)
{
	return Read(data.data(), data.size(), flags);
}

bool MediaPoint::CanRead() const
{
	return true;
}

bool MediaPoint::CanWrite() const
{
	return true;
}


} // media

} // core
