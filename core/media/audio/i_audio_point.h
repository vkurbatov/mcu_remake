#ifndef I_AUDIO_POINTH_H
#define I_AUDIO_POINTH_H

#include "media/audio/audio_format.h"

namespace core
{

namespace media
{

namespace audio
{

class IAudioPoint
{
public:

	virtual ~IAudioPoint() = default;

	virtual std::int32_t Write(const void* data, std::size_t size, const audio_format_t& audio_format, std::uint32_t options = 0) = 0;
	virtual std::int32_t Read(void* data, std::size_t size, const audio_format_t& audio_format, std::uint32_t options = 0) = 0;


};

} // audio

} // media

} // core


#endif // I_AUDIO_POINTH_H
