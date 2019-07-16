#include "audio_server.h"
#include "audio_stream.h"

#include <core-tools/logging.h>
#include "core/media/audio/audio_string_format_utils.h"

#define PTraceModule() "audio_server"

namespace core
{

namespace media
{

namespace audio
{

AudioServer::AudioServer(IAudioComposer& audio_composer)
	: m_audio_composer(audio_composer)
	, m_stream_id(media_stream_id_min)
	, m_slot_id(0)
{
	LOG(debug) << "Create audio server with audio format [" << audio_composer.GetAudioFormat() << "]" LOG_END;
}

IAudioSlot* AudioServer::GetAudioSlot(media_stream_id_t stream_id)
{
	auto stream = operator[](stream_id);

	return stream != nullptr ? get_slot(stream->GetSessionId()) : nullptr;
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

IAudioStream* AudioServer::AddStream(const session_id_t& session_id, const audio_format_t& audio_format, bool is_writer)
{

	IAudioStream* result = nullptr;

	auto audio_slot = request_slot(session_id);

	if (audio_slot != nullptr)
	{

		auto stream_id = get_stream_id();

		audio_stream_t audio_stream(
		    new AudioStream(stream_id, session_id, audio_format, *audio_slot, is_writer)
		    , [](IAudioStream * audio_stream)
		{
			delete static_cast<AudioStream*>(audio_stream);
		});

		result = audio_stream.get();

		if (result != nullptr)
		{
			m_streams.emplace(std::make_pair(stream_id, std::move(audio_stream)));
			LOG(info) << "Audio stream create success for session \'" << session_id << "\' [id = " << stream_id << ":" << is_writer << "]" LOG_END;
		}
		else
		{
			release_slot(session_id);
			LOG(error) << "Can't create audio stream for session \'" << session_id << "\' [id = " << stream_id << ":" << is_writer << "]" LOG_END;
		}

	}
	else
	{
		LOG(error) << "Can't create audio slot for session \'" << session_id << "\'" << ":" << is_writer << "]" LOG_END;
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

		auto slot_refs = release_slot(audio_stream->GetSessionId());

		m_streams.erase(stream_it);

		LOG(info) << "Audio stream [id = " << stream_id << "] remove success (slot refs = " << slot_refs << ")" LOG_END;
	}
	else
	{
		LOG(warning) << "Can't remove audio stream. Stream id = " << stream_id << " not found" LOG_END;
	}

	return result;
}

std::size_t AudioServer::Count() const
{
	return m_streams.size();
}

const audio_format_t&AudioServer::GetAudioFormat() const
{
	return m_audio_composer.GetAudioFormat();
}

bool AudioServer::SetAudioFormat(const audio_format_t& audio_format)
{
	return m_audio_composer.SetAudioFormat(audio_format);
}

media_stream_id_t AudioServer::get_stream_id()
{
	return m_stream_id++;
}

IAudioSlot* core::media::audio::AudioServer::get_slot(const session_id_t &session_id)
{
	auto it = m_sessions.find(session_id);

	return it != m_sessions.end() ? m_audio_composer[it->second.first] : nullptr;
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

		if (result != nullptr)
		{
			ref_count ++;
			LOG(info) << "Get exists audio slot by session \'" << session_id << "\' [slot_id = "
			          << result->GetSlotId() << ", refs = " << ref_count << "]" LOG_END;
		}
		else
		{
			LOG(error) << "Audio slot for session \'" << session_id << "\' [slot_id = " << slot_id << ", refs = " << ref_count << "] not found in composer"  LOG_END;
		}
	}
	else
	{
		result = m_audio_composer.QueryAudioSlot(m_slot_id);

		if (result != nullptr)
		{
			m_sessions.emplace(std::make_pair(session_id, std::make_pair(m_slot_id, 1)));
			m_slot_id++;
		}
		else
		{
			LOG(error) << "Can't get new audio slot for session \'" << session_id << "\' [slot_id = " << m_slot_id << "]" LOG_END;
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

		LOG(info) << "Release audio slot for session \'" << session_id << "\' [slot_id = " << slot_id << ", refs = " << ref_count << "]" LOG_END;
	}
	else
	{
		LOG(error) << "Can't release new audio slot for session \'" << session_id << "\'. Session not found" LOG_END;
	}

	return result;
}

} // audio

} // media

} // core
