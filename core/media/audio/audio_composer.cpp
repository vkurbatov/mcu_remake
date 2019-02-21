#include "audio_composer.h"
#include "media/audio/audio_slot.h"

namespace core
{

namespace media
{

namespace audio
{

AudioComposer::AudioComposer(const audio_format_t& audio_format, uint32_t queue_duration_ms)
	: m_audio_format(audio_format)
	, m_media_queue(audio_format.size_from_duration(queue_duration_ms))
{

}

void AudioComposer::Reset()
{
	m_media_queue.Reset();
}

std::size_t AudioComposer::Size() const
{
	return m_media_queue.Size();
}

std::size_t AudioComposer::Capacity() const
{
	return m_media_queue.Capacity();
}

IAudioSlot* AudioComposer::operator [](audio_slot_id_t media_slot_id)
{
	auto it = m_audio_slots.find(media_slot_id);

	return it != m_audio_slots.end() ? it->second.get() : nullptr;
}

const IAudioSlot* AudioComposer::operator [](audio_slot_id_t media_slot_id) const
{
	auto it = m_audio_slots.find(media_slot_id);

	return it != m_audio_slots.end() ? it->second.get() : nullptr;
}

IAudioSlot* AudioComposer::AddAudioSlot(audio_slot_id_t audio_slot_id)
{
	auto result = static_cast<AudioSlot*>(operator [](audio_slot_id));

	if (result == nullptr)
	{
		audio_slot_t audio_slot(
					new AudioSlot(m_audio_format, *m_media_queue.AddSlot(audio_slot_id))
					, [](IAudioSlot* slot) { delete static_cast<AudioSlot*>(slot); }
		);

		m_audio_slots.insert(std::make_pair(audio_slot_id, std::move(audio_slot)));
	}
	else
	{
		result->m_ref_count++;
	}

	return result;
}

std::size_t AudioComposer::RemoveAudioSlot(audio_slot_id_t audio_slot_id)
{
	std::size_t result = 0;

	auto it = m_audio_slots.find(audio_slot_id);

	if (it != m_audio_slots.end())
	{
		result = --static_cast<AudioSlot*>(it->second.get())->m_ref_count;

		if (result == 0)
		{
			m_audio_slots.erase(it);
			m_media_queue.RemoveSlot(audio_slot_id);
		}
	}

	return result;
}

const audio_format_t& AudioComposer::GetAudioFormat() const
{
	return m_audio_format;
}

bool AudioComposer::SetAudioFormat(const audio_format_t& audio_format)
{
	if (m_audio_format != audio_format)
	{
		m_audio_format = audio_format;

		for (auto& s : m_audio_slots)
		{
			static_cast<AudioSlot*>(s.second.get())->m_palyback_queue.Reset();
		}

		Reset();
	}

	return true;
}

std::size_t AudioComposer::Count() const
{
	return m_audio_slots.size();
}

} // audio

} // media

} // core
