#ifndef AUDIO_STREAM_H
#define AUDIO_STREAM_H

#include "media/audio/i_audio_stream.h"

namespace core
{

namespace media
{

namespace audio
{

class AudioStream : public IAudioStream
{

	IAudioPoint& m_audio_point;

public:
	AudioStream();
	virtual ~AudioStream() override = default;
};

} // audio

} // media

} // core

#endif // AUDIO_STREAM_H
