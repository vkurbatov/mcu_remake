#include "audio_server.h"

namespace core
{

namespace media
{

namespace audio
{

AudioServer::AudioServer(IAudioComposer& audio_composer)
	: m_audio_composer(audio_composer)
{

}

IAudioStream* AudioServer::operator [](media_stream_id_t stream_id)
{

	IAudioStream* result = nullptr;

	auto it = m_streams.find(stream_id);

	//if (it)
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

std::size_t AudioServer::Count() const
{
	return m_streams.size();
}

} // audio

} // media

} // core
