#ifndef I_AUDIO_SERVER_H
#define I_AUDIO_SERVER_H

#include "core/media/common/i_data_collection.h"
#include "core/media/audio/i_audio_stream.h"

namespace core
{

namespace media
{

namespace audio
{

#ifndef AUDIO_FORMAT_H
struct audio_format_t;
#endif

class IAudioServer: virtual public IDataCollection, virtual public IAudioFormatter
{

public:

	virtual ~IAudioServer() {}

	virtual IAudioStream* operator[](media_stream_id_t stream_id) = 0;
	virtual const IAudioStream* operator[](media_stream_id_t stream_id) const = 0;

	virtual IAudioStream* AddStream(const session_id_t& session_id, const audio_format_t& audio_format, bool is_writer = false) = 0;
	virtual bool RemoveStream(media_stream_id_t stream_id) = 0;

	virtual media_stream_id_t GetStreamIdBySessionId(const session_id_t& session_id) const = 0;
};

} // audio

} // media

} // core

#endif // I_AUDIO_SERVER_H
