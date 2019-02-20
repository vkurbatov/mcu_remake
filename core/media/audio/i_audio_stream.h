#ifndef I_AUDIO_STREAM_H
#define I_AUDIO_STREAM_H

#include "media/common/i_media_stream.h"
#include "media/common/i_data_queue.h"

#include <string>

namespace core
{

namespace media
{

namespace audio
{

using session_id_t = std::string;

#ifndef AUDIO_FORMAT_H
struct audio_format_t;
#endif

class IAudioStream : public IMediaStream, public IDataQueue
{

protected:
	virtual ~IAudioStream() = default;

public:

	virtual const session_id_t& GetSessionId() const = 0;

	virtual const audio_format_t& GetAudioFormat() const = 0;
	virtual bool SetAudioFormat(const audio_format_t& audio_format) = 0;

};

} // audio

} // media

} // core

#endif // I_AUDIO_STREAM_H
