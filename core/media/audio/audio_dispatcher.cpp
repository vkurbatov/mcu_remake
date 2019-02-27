#include "audio_dispatcher.h"
#include "media/common/timer.h"

namespace core
{

namespace media
{

namespace audio
{

AudioDispatcher::AudioDispatcher(IAudioReader& audio_reader, IAudioWriter& audio_writer, const audio_format_t& audio_format)
	: m_audio_reader(audio_reader)
	, m_audio_writer(audio_writer)
	, m_audio_format(audio_format)
	, m_is_running(false)
	, m_cycle_counter(0)
{

}

AudioDispatcher::~AudioDispatcher()
{
	Stop();
}

bool AudioDispatcher::Start(std::uint32_t duration_ms)
{

	if (m_is_running)
	{
		Stop();
	}

	m_is_running = true;
	m_dispatch_thread = std::thread(&AudioDispatcher::dispatching_proc, this, duration_ms);

	return m_dispatch_thread.joinable();
}

bool AudioDispatcher::Stop()
{
	bool result = m_is_running;

	if (m_is_running)
	{
		m_is_running = false;

		if (m_dispatch_thread.joinable())
		{
			m_dispatch_thread.join();
		}
	}

	return result;
}

bool AudioDispatcher::IsRunning() const
{
	return m_is_running;
}

uint32_t AudioDispatcher::GetCyclesCount() const
{
	return m_cycle_counter;
}

void AudioDispatcher::dispatching_proc(std::uint32_t duration_ms)
{
	m_cycle_counter = 0;

	Timer			delay;
	media_buffer_t	buffer;

	while(m_is_running)
	{
		auto size = m_audio_format.size_from_duration(duration_ms);

		if (buffer.size() < size)
		{
			buffer.resize(size);
		}

		auto result = m_audio_reader.Read(m_audio_format, buffer.data(), size);

		if (result > 0)
		{
			result = m_audio_writer.Write(m_audio_format, buffer.data(), result);
		}

		delay(duration_ms);
		m_cycle_counter++;
	}
}

} // audio

} // media

} // core
