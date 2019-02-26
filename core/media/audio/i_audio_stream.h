#ifndef I_AUDIO_STREAM_H
#define I_AUDIO_STREAM_H

#include "media/common/i_media_stream.h"
#include "media/audio/i_audio_point.h"
#include "media/audio/i_audio_formatter.h"

#include <string>

namespace core
{

namespace media
{

namespace audio
{

using session_id_t = std::string;

class IAudioStream : public IMediaStream, public IAudioPoint, public IAudioFormatter
{

protected:
	virtual ~IAudioStream() = default;

public:

	virtual const session_id_t& GetSessionId() const = 0;

};

} // audio

} // media

} // core

#endif // I_AUDIO_STREAM_H
