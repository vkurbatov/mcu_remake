#ifndef I_AUDIO_POINTH_H
#define I_AUDIO_POINTH_H

#include "core/media/common/i_media_point.h"
#include <cstdint>

namespace core
{

namespace media
{

namespace audio
{

#ifndef AUDIO_FORMAT_H
struct audio_format_t;
#endif

class IAudioReader : virtual public IMediaReadStatus
{
public:
	virtual ~IAudioReader(){}
	virtual std::int32_t Read(const audio_format_t& audio_format, void* data, std::size_t size, std::uint32_t options = 0) = 0;
};

class IAudioWriter : virtual public IMediaWriteStatus
{
public:
	virtual ~IAudioWriter(){}
	virtual std::int32_t Write(const audio_format_t& audio_format, const void* data, std::size_t size, std::uint32_t options = 0) = 0;
};

class IAudioPoint : virtual public IAudioReader, virtual public IAudioWriter
{
public:

	virtual ~IAudioPoint(){}

};

} // audio

} // media

} // core


#endif // I_AUDIO_POINTH_H
