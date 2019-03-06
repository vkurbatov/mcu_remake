#ifndef AUDIO_FORMATTER_H
#define AUDIO_FORMATTER_H

#include "media/audio/audio_format.h"

namespace core
{

namespace media
{

namespace audio
{

class IAudioFormatter
{

public:

    virtual ~IAudioFormatter() {}

	virtual const audio_format_t& GetAudioFormat() const = 0;
	virtual bool SetAudioFormat(const audio_format_t& audio_format) = 0;

};

} // audio

} // media

} // core


#endif // AUDIO_FORMATTER_H
