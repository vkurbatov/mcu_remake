#include "audio_event.h"

namespace core
{

namespace media
{

namespace audio
{

namespace tools
{

const channels::audio_channel_params_t event_channels_params = { channels::channel_direction_t::recorder, default_audio_format, 0, false };

AudioEvent::AudioEvent(const std::string& device_name
					   , std::size_t repetitions
					   , std::size_t delay_interval)
	: m_device_name(device_name)
	, m_repetitions(repetitions)
	, m_delay_interval(delay_interval)
	, m_file( event_channels_params )
{

}

std::int32_t AudioEvent::Read(const audio_format_t& audio_format, void* data, std::size_t size, std::uint32_t options)
{
	std::int32_t result = 0;

	bool is_process = !m_delay_timer.IsWait() && m_current_repetition < m_repetitions;

	if (is_process)
	{
		bool is_read = m_file.IsOpen() || m_file.Open(m_device_name);

		if (is_read)
		{
			result = m_file.Read(audio_format, data, size, options);
		}

		if (!m_file.CanRead())
		{
			m_file.Close();

			m_current_repetition++;

			m_delay_timer.Start(m_delay_interval);
		}
	}

	return result;
}

bool AudioEvent::CanRead() const
{
	return m_file.CanRead();
}

void AudioEvent::Reset(const std::string& device_name, std::size_t repetitions, std::size_t delay_interval)
{
	m_device_name = device_name;
	m_repetitions = repetitions;
	m_delay_interval = delay_interval;

	Reset();
}

void AudioEvent::Reset()
{
	m_delay_timer.Stop();

	if (m_file.IsOpen())
	{
		m_file.Close();
	}

	m_current_repetition = 0;
}

}

} // audio

} // media

} // core
