#include "media/audio/audio_resampler.h"
#include "media/audio/audio_mixer.h"
#include "audio_slot.h"

namespace core
{

namespace media
{

namespace audio
{

namespace audio_slot_utils
{
	template<typename T>
	std::size_t prepare_buffer(std::vector<T>& buffer, std::size_t new_size)
	{

		if (new_size > buffer.size())
		{
			buffer.resize(new_size);
		}

		return new_size;
	}
}

AudioSlot::AudioSlot(const audio_format_t& audio_format, IMediaSlot& media_slot, const IDataCollection& slot_collection, ISyncPoint& sync_point)
	: m_audio_format(audio_format)
	, m_media_slot(media_slot)
	, m_palyback_queue(media_slot.Capacity())
	, m_slots_collection(slot_collection)
	, m_sync_point(sync_point)
	, m_ref_count(1)
{

}

std::int32_t AudioSlot::Write(const audio_format_t& audio_format, const void* data, std::size_t size, std::uint32_t options)
{
	return internal_write(data, size, audio_format, options);
}

std::int32_t AudioSlot::Read(const audio_format_t& audio_format, void* data, std::size_t size, std::uint32_t options)
{
	return internal_read(data, size, audio_format, options);
}

audio_slot_id_t AudioSlot::GetSlotId() const
{
	return m_media_slot.GetSlotId();
}

bool AudioSlot::IsSkip() const
{
	return false;
}

std::int32_t AudioSlot::internal_write(const void* data, std::size_t size, const audio_format_t& audio_format, uint32_t options)
{

	m_sync_point.Lock();

	// prepare resample buffer and resampling

	auto output_size = m_audio_format.size_from_format(audio_format, size);

	audio_slot_utils::prepare_buffer(m_output_resampler_buffer, output_size);

	output_size = AudioResampler::Resampling(audio_format, m_audio_format, data, size, m_output_resampler_buffer.data(), output_size);


	// push playback data for future demixing

	output_size = m_palyback_queue.Push(m_output_resampler_buffer.data(), output_size);


	// prepare mixing buffer and mixing

	audio_slot_utils::prepare_buffer(m_mix_buffer, output_size);

	auto mix_size = m_media_slot.Read(m_mix_buffer.data(), output_size, true);

	if (mix_size > 0)
	{
		mix_size = AudioMixer::Mixed(m_audio_format, m_slots_collection.Count(), m_output_resampler_buffer.data(), output_size, m_mix_buffer.data(), mix_size);
	}

	// push mixed audio data into media queue and into playback queue

	output_size = m_media_slot.Push(m_mix_buffer.data(), mix_size);

	m_sync_point.Unlock();

	return audio_format.size_from_format(m_audio_format, output_size);
}

std::int32_t AudioSlot::internal_read(void* data, std::size_t size, const audio_format_t& audio_format, uint32_t options)
{

	m_sync_point.Lock();

	// prepare resampling and demixind buffer

	auto input_size = m_audio_format.size_from_format(audio_format, size);

	audio_slot_utils::prepare_buffer(m_input_resampler_buffer, input_size);

	audio_slot_utils::prepare_buffer(m_demix_buffer, input_size);


	// read audio data form media queue

	input_size = m_media_slot.Pop(m_input_resampler_buffer.data(), input_size);


	// fetch playback data and demix

	auto demix_size = m_palyback_queue.Pop(m_demix_buffer.data(), input_size);

	if (demix_size > 0)
	{
		demix_size = AudioMixer::Demixed(m_audio_format, m_slots_collection.Count(), m_demix_buffer.data(), demix_size, m_input_resampler_buffer.data(), input_size);
	}

	// resampling demix audio buffer

	input_size = AudioResampler::Resampling(audio_format, m_audio_format, m_input_resampler_buffer.data(), input_size, data, size);

	m_sync_point.Unlock();

	return audio_format.size_from_format(m_audio_format, input_size);
}

} // audio

} // media

} // core
