#include "media/common/guard_lock.h"
#include "media/audio/audio_resampler.h"
#include "media/audio/audio_mixer.h"

#include "audio_slot.h"

#include <core-tools/logging.h>
#include "media/audio/audio_string_format_utils.h"

#define PTraceModule() "audio_slot"

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

AudioSlot::AudioSlot(const audio_format_t& audio_format, IMediaSlot& media_slot, const IDataCollection& slot_collection, const ISyncPoint& sync_point, const std::uint32_t& min_jitter_ms)
	: m_audio_format(audio_format)
	, m_media_slot(media_slot)
	, m_palyback_queue(media_slot.Capacity(), true)
	, m_slots_collection(slot_collection)
	, m_sync_point(sync_point)
	, m_min_jitter_ms(min_jitter_ms)
	, m_ref_count(1)
	, m_can_slot_read(false)
{
	LOG(debug) << "Create audio slot [id = " << m_media_slot.GetSlotId()
			   << "\', format = " << audio_format
			   << "]" LOG_END;
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

void AudioSlot::Reset()
{
	m_demix_buffer.clear();
	m_mix_buffer.clear();
	m_input_resampler_buffer.clear();
	m_output_resampler_buffer.clear();
	m_palyback_queue.Reset();
	m_can_slot_read = false;
}

bool AudioSlot::CanWrite() const
{
	return m_media_slot.Capacity() > 0;
}

bool AudioSlot::CanRead() const
{
	return m_media_slot.Size() > 0;
}

int32_t AudioSlot::slot_push(const void* data, std::size_t size)
{
	return m_media_slot.Push(data, size);
}

int32_t AudioSlot::slot_pop(void* data, std::size_t size)
{
	std::int32_t result = 0;

	if (check_jitter())
	{
		result = m_media_slot.Pop(data, size);

		m_can_slot_read &= result == size;
	}

	return result;
}

bool AudioSlot::check_jitter()
{
	return m_can_slot_read = m_can_slot_read || m_audio_format.duration_ms(m_media_slot.Size()) >= m_min_jitter_ms;
}

std::int32_t AudioSlot::internal_write(const void* data, std::size_t size, const audio_format_t& audio_format, uint32_t options)
{

	GuardLock lock(m_sync_point);

	// prepare resample buffer and resampling

	auto output_size = m_audio_format.size_from_format(audio_format, size);

	audio_slot_utils::prepare_buffer(m_output_resampler_buffer, output_size);

	output_size = AudioResampler::Resampling(audio_format, m_audio_format, data, size, m_output_resampler_buffer.data(), output_size);


	// push playback data for future demixing

	output_size = m_palyback_queue.Push(m_output_resampler_buffer.data(), output_size);


	// prepare mixing buffer and mixing

	audio_slot_utils::prepare_buffer(m_mix_buffer, output_size);

	auto mix_size = m_media_slot.Read(m_mix_buffer.data(), output_size, true);

	mix_size = AudioMixer::Mixed(m_audio_format, m_slots_collection.Count(), m_output_resampler_buffer.data(), output_size, m_mix_buffer.data(), mix_size, m_mix_buffer.data(), output_size);


	// write mixed audio data into media queue and into playback queue

	output_size = slot_push(m_mix_buffer.data(), mix_size);

	return audio_format.size_from_format(m_audio_format, output_size);
}

std::int32_t AudioSlot::internal_read(void* data, std::size_t size, const audio_format_t& audio_format, uint32_t options)
{

	GuardLock lock(m_sync_point);

	// prepare resampling and demixind buffer

	auto input_size = m_audio_format.size_from_format(audio_format, size);

	audio_slot_utils::prepare_buffer(m_input_resampler_buffer, input_size);

	audio_slot_utils::prepare_buffer(m_demix_buffer, input_size);


	// read audio data form media queue

	input_size = slot_pop(m_input_resampler_buffer.data(), input_size);


	// fetch playback data and demix

	auto demix_size = m_palyback_queue.Pop(m_demix_buffer.data(), input_size);

	demix_size = AudioMixer::Demixed(m_audio_format, m_slots_collection.Count(), m_demix_buffer.data(), demix_size, m_input_resampler_buffer.data(), input_size);


	// resampling demix audio buffer

	// input_size = AudioResampler::Resampling(audio_format, m_audio_format, m_input_resampler_buffer.data(), input_size, data, size);

	input_size = AudioResampler::Resampling(m_audio_format, audio_format, m_input_resampler_buffer.data(), input_size, data, size);

	return input_size;
}

} // audio

} // media

} // core
