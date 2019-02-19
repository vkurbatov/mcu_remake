#ifndef I_MEDIA_STREAM_H
#define I_MEDIA_STREAM_H

#include <cstdint>

namespace core
{

namespace media
{

using media_stream_id_t = std::uint32_t;

class IMediaStream
{
public:
	virtual ~IMediaStream() = default;

	virtual media_stream_id_t GetStreamId() const = 0;

};

} // media

} // core

#endif // I_MEDIA_STREAM_H
