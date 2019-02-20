#include "audio_server.h"

namespace core
{

namespace media
{

namespace audio
{

AudioServer::AudioServer()
{

}

IAudioStream* AudioServer::operator [](media_stream_id_t stream_id)
{

}

const IAudioStream* AudioServer::operator [](media_stream_id_t stream_id) const
{

}

IAudioStream* AudioServer::AddStream(const audio_format_t& audio_format, const session_id_t& session_id)
{

}

bool AudioServer::RemoveStream(media_stream_id_t stream_id)
{

}

} // audio

} // media

} // core
