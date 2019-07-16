#include "core/media/common/guard_lock.h"
#include "core/media/audio/audio_resampler.h"
#include "core/media/audio/audio_mixer.h"

#include "audio_slot.h"

#include <core-tools/logging.h>
#include "core/media/audio/audio_string_format_utils.h"

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
inline std::size_t prepare_buffer(std::vector<T>& buffer, std::size_t new_size)
{

	if (new_size > buffer.size())
	{
		buffer.resize(new_size);
	}

	return new_size;
}
}

AudioSlot::AudioSlot(const audio_format_t& audio_format
                     , IMediaSlot& media_slot
                     , const IDataCollection& slot_collection
                     , const ISyncPoint& sync_point
                     , const std::uint32_t& compose_window_ms
                     , const std::uint32_t& read_delay_ms
                     , const std::uint32_t& dead_zone_ms)
	: m_audio_format(audio_format)
	, m_media_slot(media_slot)
	, m_palyback_queue(media_slot.Capacity(), false)
	, m_slots_collection(slot_collection)
	, m_sync_point(sync_point)
	, m_compose_window_ms(compose_window_ms)
	, m_read_delay_ms(read_delay_ms)
	, m_dead_zone_ms(dead_zone_ms)
	, m_ref_count(1)
	, m_drop_bytes(audio_format.size_from_duration(compose_window_ms + dead_zone_ms + read_delay_ms))
{
	LOG(debug) << "Create audio slot [id = " << m_media_slot.GetSlotId()
	           << "\', format = " << audio_format
	           << "]" LOG_END;
}

AudioSlot::~AudioSlot()
{
	LOG(debug) << "Destroy audio slot [id = " << m_media_slot.GetSlotId() << "]" LOG_END;
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
	m_drop_bytes = m_audio_format.size_from_duration(m_compose_window_ms + m_dead_zone_ms + m_read_delay_ms);
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

	result = m_media_slot.Pop(data, size);

	return result;
}

bool AudioSlot::prepare_write(std::size_t write_size)
{

	auto write_jitter = m_audio_format.duration_ms(m_media_slot.WriteJitter());

	// TODO: джиттер для записи и чтения нужно разделять

	bool is_syncronize = write_jitter > m_compose_window_ms;

	if (is_syncronize)
	{
		LOG(warning) << "SLOT #" << GetSlotId() << ": write jitter exceeded by " << write_jitter << " ms. Do syncronize stream" LOG_END;
		syncronize();
	}

	return is_syncronize;
}

bool AudioSlot::prepare_read(std::size_t read_size)
{
	auto r_jitter = m_media_slot.ReadJitter();
	auto capacity = m_media_slot.Capacity();

	if (r_jitter >= capacity)
	{
		auto lagged_ms = m_audio_format.duration_ms(r_jitter - capacity);
		LOG(warning) << "SLOT #" << GetSlotId() << ": read jitter lagged behind by " << lagged_ms << ". Do syncronize stream." LOG_END;
		syncronize();
	}
	else
	{
		if (is_drop())
		{
			m_drop_bytes -= std::min(read_size, m_drop_bytes);
		}
		else
		{
			auto read_jitter_ms = m_audio_format.duration_ms(m_media_slot.ReadJitter());

			bool is_syncronize = read_jitter_ms <= (m_compose_window_ms + m_dead_zone_ms);

			if (is_syncronize)
			{
				auto delay = (m_compose_window_ms + m_dead_zone_ms) - (read_jitter_ms);
				LOG(warning) << "SLOT #" << GetSlotId() << ": read delay exceeded by " << delay << " ms. Do syncronize read stream" LOG_END;
				m_drop_bytes = m_audio_format.size_from_duration(delay + m_read_delay_ms);
			}

		}
	}

	return is_drop();
}

bool audio::AudioSlot::is_drop() const
{
	return m_drop_bytes > 0;
}


std::int32_t AudioSlot::internal_write(const void* data, std::size_t size, const audio_format_t& audio_format, uint32_t options)
{

	GuardLock lock(m_sync_point);

	// prepare resample buffer and resampling

	auto real_output_size = m_audio_format.size_from_format(audio_format, size);

	audio_slot_utils::prepare_buffer(m_output_resampler_buffer, real_output_size);

	real_output_size = AudioResampler::Resampling(audio_format, m_audio_format, data, size, m_output_resampler_buffer.data(), real_output_size);

	prepare_write(real_output_size);

	// push playback data for future demixing

	auto output_size = m_palyback_queue.Push(m_output_resampler_buffer.data(), real_output_size);


	// LOG(debug) << "Write slot #" << GetSlotId() << ": push to playback buffer " << output_size << " bytes of " << real_output_size LOG_END;

	// prepare mixing buffer and mixing

	audio_slot_utils::prepare_buffer(m_mix_buffer, output_size);

	auto mix_size = m_media_slot.Read(m_mix_buffer.data(), output_size, true);


	mix_size = AudioMixer::Mixed(m_audio_format, m_slots_collection.Count(), m_output_resampler_buffer.data(), output_size, m_mix_buffer.data(), mix_size, m_mix_buffer.data(), output_size);


	// write mixed audio data into media queue and into playback queue

	output_size = slot_push(m_mix_buffer.data(), mix_size);

	// LOG(debug) << "Write slot #" << GetSlotId() << ": push to media queue " << output_size << " bytes of " << mix_size LOG_END;

	return audio_format.size_from_format(m_audio_format, output_size);
}

std::int32_t AudioSlot::internal_read(void* data, std::size_t size, const audio_format_t& audio_format, uint32_t options)
{

	GuardLock lock(m_sync_point);

	// prepare resampling and demixind buffer

	auto real_input_size = m_audio_format.size_from_format(audio_format, size);

	prepare_read(real_input_size);

	if (!is_drop())
	{

		audio_slot_utils::prepare_buffer(m_input_resampler_buffer, real_input_size);

		audio_slot_utils::prepare_buffer(m_demix_buffer, real_input_size);


		// read audio data form media queue

		auto input_size = slot_pop(m_input_resampler_buffer.data(), real_input_size);


		// fetch playback data and demix

		auto demix_size = m_palyback_queue.Pop(m_demix_buffer.data(), input_size);

		// LOG(debug) << "Read slot #" << GetSlotId() << ": pop from media queue " << demix_size << " bytes of " << input_size LOG_END;

		demix_size = AudioMixer::Demixed(m_audio_format, m_slots_collection.Count(), m_demix_buffer.data(), demix_size, m_input_resampler_buffer.data(), input_size);

		// resampling demix audio buffer


		size = AudioResampler::Resampling(m_audio_format, audio_format, m_input_resampler_buffer.data(), input_size, data, size);

	}
	else
	{
		size = 0;
	}

	return size;
}

void core::media::audio::AudioSlot::syncronize()
{
	m_palyback_queue.Reset();
	m_media_slot.Reset();
	m_drop_bytes = m_audio_format.size_from_duration(m_compose_window_ms + m_dead_zone_ms + m_read_delay_ms);
}

} // audio

} // media

} // core
