#ifndef I_AUDIO_SERVER_H
#define I_AUDIO_SERVER_H

#include "media/audio/i_audio_stream.h"

namespace core
{

namespace media
{

namespace audio
{

class IAudioServer
{

public:

	virtual ~IAudioServer() = default;

	virtual IAudioStream* operator[](media_stream_id_t stream_id) = 0;
	virtual const IAudioStream* operator[](media_stream_id_t stream_id) const = 0;

	virtual AddStream(media_stream_id_t stream_id, IAudioPoint& audio_point);
};

} // audio

} // media

} // core

#endif // I_AUDIO_SERVER_H
