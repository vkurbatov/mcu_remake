#include "audio_server.h"
#include "audio_stream.h"

namespace core
{

namespace media
{

namespace audio
{

AudioServer::AudioServer(IAudioComposer& audio_composer)
	: m_audio_composer(audio_composer)
	, m_stream_id(0)
	, m_slot_id(0)
{

}

IAudioStream* AudioServer::operator [](media_stream_id_t stream_id)
{
	auto it = m_streams.find(stream_id);

	return it != m_streams.end() ? it->second.get() : nullptr;
}

const IAudioStream* AudioServer::operator [](media_stream_id_t stream_id) const
{
	auto it = m_streams.find(stream_id);

	return it != m_streams.end() ? it->second.get() : nullptr;
}

IAudioStream* AudioServer::AddStream(const audio_format_t& audio_format, const session_id_t& session_id, bool is_writer)
{

	IAudioStream* result = nullptr;

	auto audio_slot = request_slot(session_id);

	if (audio_slot != nullptr)
	{

		auto stream_id = get_stream_id();

		audio_stream_t audio_stream(
					new AudioStream(stream_id, session_id, audio_format, *audio_slot)
					, [](IAudioStream* audio_stream) { return static_cast<AudioStream*>(audio_stream); } );

		result = audio_stream.get();

		m_streams.emplace( std::make_pair( stream_id, std::move(audio_stream) ) );

	}

	return result;
}

bool AudioServer::RemoveStream(media_stream_id_t stream_id)
{

	auto stream_it = m_streams.find(stream_id);

	bool result = stream_it != m_streams.end();

	if (result)
	{
		auto audio_stream = stream_it->second.get();

		release_slot(audio_stream->GetSessionId());

		m_streams.erase(stream_it);
	}

	return result;
}

std::size_t AudioServer::Count() const
{
	return m_streams.size();
}

media_stream_id_t AudioServer::get_stream_id()
{
	return m_stream_id++;
}

IAudioSlot *AudioServer::request_slot(const session_id_t &session_id)
{
	IAudioSlot* result = nullptr;

	auto it = m_sessions.find(session_id);

	if (it != m_sessions.end())
	{
		auto& slot_id = it->second.first;
		auto& ref_count = it->second.second;

		result = m_audio_composer[slot_id];

		ref_count += static_cast<std::size_t>( result != nullptr );

	}
	else
	{
		result = m_audio_composer.QueryAudioSlot(m_slot_id);

		if (result != nullptr)
		{
			m_sessions.emplace( std::make_pair( session_id, std::make_pair(m_slot_id, 1) ) );
			m_slot_id++;
		}
	}

	return result;
}

std::size_t AudioServer::release_slot(const session_id_t &session_id)
{
	std::size_t result = 0;

	auto it = m_sessions.find(session_id);

	if (it != m_sessions.end())
	{
		auto& slot_id = it->second.first;
		auto& ref_count = it->second.second;

		result = (ref_count -= static_cast<std::size_t>(ref_count != 0));

		if (result == 0)
		{
			m_audio_composer.ReleaseAudioSlot(slot_id);
			m_sessions.erase(it);
		}
	}

	return result;
}

} // audio

} // media

} // core
