#include "audio_composer.h"
#include "media/audio/audio_slot.h"

#include <algorithm>

namespace core
{

namespace media
{

namespace audio
{

AudioComposer::AudioComposer(const audio_format_t& audio_format, IMediaQueue& media_queue, std::uint32_t min_jitter_ms)
	: m_audio_format(audio_format)
	, m_media_queue(media_queue)
	, m_slot_collection(m_audio_slots)
	, m_min_jitter_ms(min_jitter_ms)
{

}

void AudioComposer::Reset()
{
	m_media_queue.Reset();
	for (auto& s : m_audio_slots)
	{
		s.second->Reset();
	}
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
	return operator[](media_slot_id);
	/*auto it = m_audio_slots.find(media_slot_id);

	return it != m_audio_slots.end() ? it->second.get() : nullptr;*/
}

IAudioSlot* AudioComposer::QueryAudioSlot(audio_slot_id_t audio_slot_id)
{
	auto result = static_cast<AudioSlot*>(operator [](audio_slot_id));

	if (result == nullptr)
	{
		auto media_slot = m_media_queue.QuerySlot(audio_slot_id);

		if (media_slot != nullptr)
		{
			audio_slot_t audio_slot(
						new AudioSlot(m_audio_format, *media_slot, m_slot_collection, *this, m_min_jitter_ms)
						, [](IAudioSlot* slot) { delete static_cast<AudioSlot*>(slot); }
			);

			result = static_cast<AudioSlot*>(audio_slot.get());

			if (result != nullptr)
			{
				m_audio_slots.insert(std::make_pair(audio_slot_id, std::move(audio_slot)));
			}
		}
	}
	else
	{
		result->m_ref_count++;
	}

	return result;
}

std::size_t AudioComposer::ReleaseAudioSlot(audio_slot_id_t audio_slot_id)
{
	std::size_t result = 0;

	auto it = m_audio_slots.find(audio_slot_id);

	if (it != m_audio_slots.end())
	{

		auto& slot = static_cast<AudioSlot&>(*it->second);

		slot.m_ref_count -= static_cast<std::size_t>(slot.m_ref_count > 0);

		if ( (result = slot.m_ref_count) == 0)
		{
			m_audio_slots.erase(it);
			m_media_queue.ReleaseSlot(audio_slot_id);
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

void AudioComposer::Lock() const
{
	m_mutex.lock();
}

void AudioComposer::Unlock() const
{
	m_mutex.unlock();
}

AudioComposer::SlotCollectionWrapper::SlotCollectionWrapper(AudioComposer::audio_slot_map_t& audio_slots)
	: m_audio_slots(audio_slots)
{

}

std::size_t AudioComposer::SlotCollectionWrapper::Count() const
{
	return std::count_if(m_audio_slots.begin(), m_audio_slots.end(), [](const std::pair<audio_slot_id_t, audio_slot_t>& it) { return it.second->IsSkip() == false; });
}

} // audio

} // media

} // core
