#include "audio_composer.h"

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

}

std::size_t AudioComposer::Size() const
{

}

std::size_t AudioComposer::Capacity() const
{

}

IAudioSlot* AudioComposer::operator [](audio_slot_id_t media_slot_id)
{
	auto it = m_audio_slots.find(media_slot_id);

	return it != m_audio_slots.end() ? it->second.audio_slot.get() : nullptr;
}

const IAudioSlot* AudioComposer::operator [](audio_slot_id_t media_slot_id) const
{
	auto it = m_audio_slots.find(media_slot_id);

	return it != m_audio_slots.end() ? it->second.audio_slot.get() : nullptr;
}

IAudioSlot* AudioComposer::AddAudioSlot(audio_slot_id_t audio_slot_id)
{
	IAudioSlot* result = nullptr;

	auto it = m_audio_slots.find(media_slot_id);

	if (it != m_audio_slots.end())
	{
		it->
	}
	else
	{

	}

	return result;
}

uint32_t AudioComposer::RemoveAudioSlot(audio_slot_id_t audio_slot_id)
{

}

const audio_format_t& AudioComposer::GetAudioFormat() const
{

}

bool AudioComposer::SetAudioFormat(const audio_format_t& audio_format)
{

}

} // audio

} // media

} // core
