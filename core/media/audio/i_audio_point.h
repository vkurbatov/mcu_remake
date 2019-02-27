#ifndef I_AUDIO_POINTH_H
#define I_AUDIO_POINTH_H

// #include "media/audio/audio_format.h"
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

class IAudioReader
{
public:
	virtual ~IAudioReader() = default;
	virtual std::int32_t Read(const audio_format_t& audio_format, void* data, std::size_t size, std::uint32_t options = 0) = 0;
};

class IAudioWriter
{
public:
	virtual ~IAudioWriter() = default;
	virtual std::int32_t Write(const audio_format_t& audio_format, const void* data, std::size_t size, std::uint32_t options = 0) = 0;
};

class IAudioPoint : public IAudioReader, public IAudioWriter
{
public:

	virtual ~IAudioPoint() = default;

};

} // audio

} // media

} // core


#endif // I_AUDIO_POINTH_H
