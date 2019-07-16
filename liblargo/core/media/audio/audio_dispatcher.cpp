#include "audio_dispatcher.h"
#include "core/media/common/delay_timer.h"

#include <core-tools/logging.h>
#include "core/media/audio/audio_string_format_utils.h"

#define PTraceModule() "audio_dispatcher"

#include <cstring>

namespace core
{

namespace media
{

namespace audio
{

AudioDispatcher::AudioDispatcher(IAudioReader& audio_reader,
                                 IAudioWriter& audio_writer,
                                 const audio_format_t& audio_format,
                                 bool is_strong_sizes)
	: m_audio_reader(audio_reader)
	, m_audio_writer(audio_writer)
	, m_audio_format(audio_format)
	, m_is_running(false)
	, m_cycle_counter(0)
	, m_is_strong_sizes(is_strong_sizes)
{
	LOG(debug) << "Create audio dispatcher with format [" << audio_format << "] with "
	           << (is_strong_sizes ? "strong" : "weak") << " size control" LOG_END;
}

AudioDispatcher::~AudioDispatcher()
{
	Stop();
	LOG(debug) << "Destroy audio dispatcher with format [" << m_audio_format << "]" LOG_END;
}

bool AudioDispatcher::Start(std::uint32_t duration_ms)
{

	LOG(info) << (m_is_running ? "Recreated" : "Created") << " dispatcher thread with duration " << duration_ms LOG_END;

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

		LOG(info) << "Stop dispatcher thread" LOG_END;

		if (m_dispatch_thread.joinable())
		{
			m_dispatch_thread.join();
		}
	}
	else
	{
		LOG(warning) << "Dispatcher thread already stopped" LOG_END;
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

	DelayTimer			delay;
	media_buffer_t		buffer;

	LOG(info) << "Started audio dispatcher [format = " << m_audio_format << ", duration " << duration_ms << "ms]" LOG_END;

	while (m_is_running)
	{
		auto size = m_audio_format.size_from_duration(duration_ms);

		if (buffer.size() < size)
		{
			buffer.resize(size);
		}

		if (m_audio_reader.CanRead())
		{
			auto result = m_audio_reader.Read(m_audio_format, buffer.data(), size);

			if (result < 0)
			{
				LOG(warning) << "Audio dispatcher read error = " << result LOG_END;
				result = 0;
			}

			if (result < size && m_is_strong_sizes)
			{
				std::memset(buffer.data() + result, 0, size - result);
				result = size;
			}

			if (result > 0)
			{
				result = m_audio_writer.Write(m_audio_format, buffer.data(), result);
			}
			else
			{
				result = 0;
			}
		}

		delay(duration_ms);
		m_cycle_counter++;
	}

	LOG(info) << "Stopped audio dispatcher [format = " << m_audio_format << ", duration " << duration_ms << "ms]"  LOG_END;
}

} // audio

} // media

} // core
