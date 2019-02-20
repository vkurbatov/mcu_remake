#ifndef I_AUDIO_MIXER_H
#define I_AUDIO_MIXER_H

#include <cstdint>

namespace core
{

namespace media
{

namespace audio
{

#ifndef
struct audio_format_t;
#endif

class IAudioMixer
{
public:
	std::size_t	Mixed(const audio_format_t, std::size_t const void );

};

} // audio

} // media

} // core

#endif // I_AUDIO_MIXER_H
