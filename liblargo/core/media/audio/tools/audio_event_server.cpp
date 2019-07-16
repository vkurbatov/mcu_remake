#include "audio_event_server.h"

#include "core/media/audio/audio_mixer.h"
#include "core/media/common/guard_lock.h"

#include <algorithm>
#include <cstring>

#include <core-tools/logging.h>
#include "core/media/audio/audio_string_format_utils.h"

#define PTraceModule() "audio_event_server"

namespace core
{

namespace media
{

namespace audio
{

namespace tools
{

const channels::audio_channel_params_t event_channels_params = { channels::channel_direction_t::recorder, default_audio_format, 0, false };

AudioEventServer::AudioEvent::AudioEvent(const std::string &file_name
        , uint32_t times
        , uint32_t interval)
	: m_file_name(file_name)
	, m_times(times)
	, m_inteval(interval)
	, m_file(event_channels_params)
	, m_step(0)
	, m_ref_count(0)
{
	LOG(debug) << "Create audio event [\'" << file_name << "\'/" << times << "/" << interval LOG_END;
}

void AudioEventServer::AudioEvent::Reset(const std::string& file_name, uint32_t times, uint32_t interval)
{
	m_file_name = file_name;
	m_times = times;
	m_inteval = interval;

	Reset();
}

void AudioEventServer::AudioEvent::Reset()
{
	m_delay_timer.Stop();
	m_step = 0;
	m_ref_count = 0;
}

void AudioEventServer::AudioEvent::Start()
{
	m_ref_count++;
	m_step = 0;
}

void AudioEventServer::AudioEvent::Stop(bool force)
{
	m_ref_count -= static_cast<std::size_t>(m_ref_count > 0);

	if (force || m_ref_count == 0)
	{
		Reset();
	}
}

bool AudioEventServer::AudioEvent::IsPlay() const
{
	return m_ref_count > 0 && m_step < m_times;
}

std::int32_t AudioEventServer::AudioEvent::Read(const audio_format_t &audio_format, void *data, std::size_t size, std::uint32_t options)
{
	std::int32_t result = 0;

	if (IsPlay())
	{
		if (!m_delay_timer.IsWait())
		{
			if ((m_file.IsOpen() || m_file.Open(m_file_name)))
			{
				result = m_file.Read(audio_format, data, size, options);
			}

			if (!m_file.CanRead())
			{
				m_file.Close();
				m_delay_timer.Start(m_inteval);
				m_step++;
			}
		}
	}
	else
	{
		Reset();
	}

	return result;
}

bool AudioEventServer::AudioEvent::CanRead() const
{
	return IsPlay() && m_file.CanRead();
}

AudioEventServer::AudioEventServer(IAudioWriter& audio_writer, const audio_format_t &audio_format, uint32_t duration_ms, IProcessStateNotifier* state_notifier)
	: m_audio_writer(audio_writer)
	, m_audio_format(audio_format)
	, m_duration_ms(duration_ms)
	, m_state_notifier(state_notifier)
	, m_running(false)
	, m_process_state(ProcessState::init)
{
	LOG(debug) << "Create audio event server [" << audio_format << "/" << m_duration_ms << "]" LOG_END;
}

AudioEventServer::~AudioEventServer()
{
	Stop();
}

std::size_t AudioEventServer::Count() const
{
	return m_events.size();
}

bool AudioEventServer::AddEvent(const std::string &event_name, const std::string& file_name, uint32_t times, uint32_t interval)
{
	GuardLock lock(m_sync_point);

	auto it = m_events.find(event_name);

	if (it != m_events.end())
	{
		it->second.Reset(file_name, times, interval);
		LOG(info) << "Change exists audio event \'" << event_name << "\' [" << file_name << "/" << times << "/" << interval << "]" LOG_END;
	}
	else
	{
		// C++11 method for emplace complex object (see C++11 documentation)

		m_events.emplace(std::piecewise_construct
		                 , std::forward_as_tuple(event_name)
		                 , std::forward_as_tuple(file_name, times, interval));

		LOG(info) << "Register new audio event \'" << event_name << "\' [" << file_name << "/" << times << "/" << interval << "]" LOG_END;
	}

	return true;
}

bool AudioEventServer::RemoveEvent(const std::string &event_name)
{
	GuardLock lock(m_sync_point);

	auto it = m_events.find(event_name);

	bool result = it != m_events.end();

	if (result)
	{
		m_events.erase(it);
		LOG(info) << "Audio event \'" << event_name << "\' removed success" LOG_END;
	}
	else
	{
		LOG(error) << "Audio event \'" << event_name << "\' not found" LOG_END;
	}

	return result;
}

bool AudioEventServer::PlayEvent(const std::string &event_name)
{
	GuardLock lock(m_sync_point);

	auto it = m_events.find(event_name);

	bool result = it != m_events.end();

	if (result)
	{

		LOG(info) << "Playing audio event \'" << event_name << "\'." LOG_END;

		it->second.Start();

		bool flag = false;

		if (m_running.compare_exchange_strong(flag, true))
		{
			if (m_event_thread.joinable())
			{
				m_event_thread.join();
			}

			m_event_thread = std::thread(&AudioEventServer::event_proc, this);
		}
	}
	else
	{
		LOG(error) << "Audio event \'" << event_name << "\' not found" LOG_END;
	}

	return result;
}

bool AudioEventServer::StopEvent(const std::string &event_name, bool force)
{
	GuardLock lock(m_sync_point);

	auto it = m_events.find(event_name);

	bool result = it != m_events.end();

	if (result)
	{
		LOG(info) << "Stopped audio event \'" << event_name << "\'" LOG_END;

		it->second.Stop(force);
	}
	else
	{
		LOG(error) << "Audio event \'" << event_name << "\' not found" LOG_END;
	}

	return result;
}

void AudioEventServer::Stop()
{

	if (m_running == true)
	{
		m_running = false;

		if (m_event_thread.joinable())
		{
			m_event_thread.join();
		}
	}

	for (auto& s : m_events)
	{
		s.second.Reset();
	}
}

bool AudioEventServer::IsRunning() const
{
	return m_running;
}

void AudioEventServer::event_proc()
{

	auto size = m_audio_format.size_from_duration(m_duration_ms);

	m_audio_buffer.resize(size);
	m_mix_buffer.resize(size);

	DelayTimer	delay_timer;

	LOG(info) << "Started audio event thread" LOG_END;

	set_state(ProcessState::start);

	while (m_running)
	{

		std::size_t event_count = 0;

		{
			GuardLock lock(m_sync_point);

			for (const auto& e : m_events)
			{
				event_count += static_cast<size_t>(e.second.IsPlay());
			}

			if (event_count > 0)
			{
				std::memset(m_mix_buffer.data(), 0, size);

				std::int32_t max_size = 0;

				for (auto& e : m_events)
				{
					if (e.second.IsPlay())
					{
						auto read_size = e.second.Read(m_audio_format, m_audio_buffer.data(), size, 0);

						if (read_size > 0)
						{
							read_size = AudioMixer::Mixed(m_audio_format, event_count
							                              , m_audio_buffer.data(), read_size
							                              , m_mix_buffer.data(), size);

							max_size = std::max(max_size, read_size);
						}
					}
				}

				if (max_size > 0)
				{
					m_volume_controller(m_audio_format.sample_format, m_mix_buffer.data(), max_size);
					m_audio_writer.Write(m_audio_format, m_mix_buffer.data(), max_size);
				}
			}
			else
			{
				m_running = false;
			}
		}

		if (m_running)
		{
			delay_timer(m_duration_ms);
		}
	}

	LOG(info) << "Stopped audio event thread" LOG_END;

	set_state(ProcessState::stop);
}

void AudioEventServer::set_state(const ProcessState& process_state)
{
	if (process_state != m_process_state)
	{
		if (m_state_notifier != nullptr)
		{
			m_state_notifier->StateChangeNotify(process_state, m_process_state, this);
		}

		m_process_state = process_state;
	}
}

uint32_t AudioEventServer::GetVolume() const
{
	return m_volume_controller.GetVolume();
}

void AudioEventServer::SetVolume(std::uint32_t volume)
{
	m_volume_controller.SetVolume(volume);
}

bool AudioEventServer::IsMute() const
{
	return m_volume_controller.IsMute();
}

void AudioEventServer::SetMute(bool mute)
{
	m_volume_controller.SetMute(mute);
}


}

} // audio

} // media

} // core
