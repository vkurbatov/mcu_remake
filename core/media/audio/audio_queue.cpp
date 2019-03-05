#include "audio_queue.h"
#include "media/common/guard_lock.h"

namespace core
{

namespace media
{

namespace audio
{

AudioQueue::AudioQueue(const audio_format_t& audio_format
					   , uint32_t queue_duration_ms
					   , uint32_t jitter_ms
					   , bool thread_safe)
	: m_resampler(audio_format)
	, m_queue_duration_ms(queue_duration_ms)
	, m_jitter_ms(jitter_ms)
	, m_can_read(false)
	, m_sync_point(!thread_safe)
	, m_data_queue(audio_format.size_from_duration(queue_duration_ms))
{

}

bool AudioQueue::check_jitter()
{
	return m_can_read |= GetAudioFormat().duration_ms(m_data_queue.Size()) >= m_jitter_ms;
}

std::int32_t AudioQueue::Write(const audio_format_t& audio_format, const void* data, std::size_t size, uint32_t options)
{
	GuardLock lock(*this);

	std::int32_t result = 0;

	if (audio_format == GetAudioFormat())
	{
		result = m_data_queue.Push(data, size);
	}
	else
	{
		auto output_size = GetAudioFormat().size_from_format(audio_format, size);

		if (output_size > m_output_resampler_buffer.size())
		{
			m_output_resampler_buffer.resize(output_size);
		}

		output_size = m_resampler(audio_format, data, size, m_output_resampler_buffer.data(), output_size);

		result = m_data_queue.Push(m_output_resampler_buffer.data(), output_size);
	}

	return result;
}

std::int32_t AudioQueue::Read(const audio_format_t& audio_format, void* data, std::size_t size, uint32_t options)
{

	GuardLock lock(*this);

	std::int32_t result = 0;

	if (check_jitter())
	{

		bool read_only = (options & audio_queue_option_peek) != 0;

		if (audio_format == GetAudioFormat())
		{
			result = read_only ? m_data_queue.Read(data, size) : m_data_queue.Pop(data, size);
		}
		else
		{
			auto input_size = GetAudioFormat().size_from_format(audio_format, size);

			if (input_size > m_input_resampler_buffer.size())
			{
				m_input_resampler_buffer.resize(input_size);
			}

			input_size = read_only
					? m_data_queue.Read(m_input_resampler_buffer.data(), input_size)
					: m_data_queue.Pop(m_input_resampler_buffer.data(), input_size);

			result = m_resampler(audio_format, m_input_resampler_buffer.data(), input_size, data, size, true);
		}

		m_can_read &= (m_data_queue.Size() > 0);

	}
	return result;
}

const audio_format_t& AudioQueue::GetAudioFormat() const
{
	return m_resampler.GetAudioFormat();
}

bool AudioQueue::SetAudioFormat(const audio_format_t& audio_format)
{
	bool result = false;

	if (m_resampler.GetAudioFormat() != audio_format)
	{
		result = m_resampler.SetAudioFormat(audio_format);
		Reset();
	}

	return result;
}

void AudioQueue::Reset()
{
	m_data_queue.Reset();
	m_can_read = false;
}

std::size_t AudioQueue::Size() const
{
	return m_data_queue.Size();
}

std::size_t AudioQueue::Capacity() const
{
	return m_data_queue.Capacity();
}

void AudioQueue::Lock() const
{
	m_sync_point.Lock();
}

void AudioQueue::Unlock() const
{
	m_sync_point.Unlock();
}

bool AudioQueue::CanWrite() const
{
	return Capacity() > 0;
}

bool AudioQueue::CanRead() const
{
	return Size() > 0;
}

} // audio

} // media

} // core
