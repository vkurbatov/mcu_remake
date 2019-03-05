#include "audio_channel_worker.h"
#include "media/common/delay_timer.h"
#include "media/common/guard_lock.h"

#include <core-tools/logging.h>
#include "media/audio/audio_string_format_utils.h"

#define PTraceModule() "audio_channel_worker"

namespace core
{

namespace media
{

namespace audio
{

namespace channels
{

AudioChannelWorker::AudioChannelWorker(IAudoChannel& audio_channel, IMediaPoint& media_point, std::size_t queue_size)
	: m_audio_channel(audio_channel)
	, m_media_point(media_point)
	, m_audio_queue(queue_size)
	, m_running(false)
{

}

AudioChannelWorker::~AudioChannelWorker()
{
	Close();
}

bool AudioChannelWorker::Open(const std::string& device_name)
{
	if (IsOpen())
	{
		Close();
	}

	if (!m_running)
	{

		LOG(info) << "Opened worker thread for device \'" << device_name << "\' success" LOG_END;

		m_running = true;
		m_dispatch_thread = std::thread(&AudioChannelWorker::audio_dispatcher_proc, this, device_name);

	}
	else
	{
		LOG(warning) << "The worker is alredy running. Can't create worker for device \'" << device_name << "\'" LOG_END;
	}

	return m_running;
}

bool AudioChannelWorker::Close()
{
	bool result = m_running;

	if (result)
	{
		m_running = false;

		LOG(info) << "Stopped worker thread" LOG_END;

		if (m_dispatch_thread.joinable())
		{
			m_dispatch_thread.join();
		}
	}
	else
	{
		LOG(warning) << "Worker thread is already stopped" LOG_END;
	}

	m_audio_queue.Reset();

	return result;
}

bool AudioChannelWorker::IsOpen() const
{
	return m_running;
}

bool AudioChannelWorker::IsRecorder() const
{
	return m_audio_channel.IsRecorder();
}

bool AudioChannelWorker::IsPlayback() const
{
	return m_audio_channel.IsPlayback();
}

const std::string& AudioChannelWorker::GetName() const
{
	return m_audio_channel.GetName();
}

int32_t AudioChannelWorker::internal_write(const void* data, std::size_t size, uint32_t options)
{
	GuardLock lock(m_sync_point);
	return m_audio_queue.Push(data, size);
}

int32_t AudioChannelWorker::internal_read(void* data, std::size_t size, uint32_t options)
{
	GuardLock lock(m_sync_point);
	return m_audio_queue.Pop(data, size);
}

const audio_channel_params_t& AudioChannelWorker::internal_get_audio_params() const
{

	return m_audio_channel.GetAudioParams();

}

bool AudioChannelWorker::internal_set_audio_params(const audio_channel_params_t& audio_params)
{
	bool result = !IsOpen();

	if (result && audio_params != m_audio_channel.GetAudioParams())
	{
		{
			GuardLock lock(m_sync_point);

			m_audio_queue.Reset();
		}

		result = m_audio_channel.SetAudioParams(audio_params);
	}

	return result;
}

void AudioChannelWorker::audio_dispatcher_proc(const std::string device_name)
{

	DelayTimer timer;

	media_buffer_t buffer( m_audio_channel.GetAudioParams().buffer_size() );

	auto delay_ms = m_audio_channel.GetAudioParams().buffer_duration_ms;

	LOG(info) << "Started audio worker for device \'" << device_name << "\', duration = " << delay_ms << "ms" LOG_END;

	if (m_audio_channel.Open(device_name))
	{
		while (m_running && m_audio_channel.IsOpen())
		{
			if (IsPlayback())
			{
				std::int32_t result = 0;
				{
					GuardLock lock(m_sync_point);
					result = m_audio_queue.Pop(buffer.data(), buffer.size());
				}

				if (result > 0)
				{
					m_media_point.Write(buffer.data(), result);
				}
			}

			if (IsRecorder())
			{
				auto result = m_media_point.Read(buffer.data(), buffer.size());

				if (result > 0)
				{
					GuardLock lock(m_sync_point);
					m_audio_queue.Push(buffer.data(), result);
				}
			}

			if (m_running)
			{
				timer(delay_ms);
			}
		}

		m_audio_channel.Close();

		LOG(info) << "Worker for device \'" << device_name << "\' stopped. " LOG_END;
	}
	else
	{
		LOG(error) << "Can't open device \'" << device_name << "\'. Worker stopped." LOG_END;
	}
}

bool AudioChannelWorker::CanWrite() const
{
	return IsOpen() && IsPlayback();
}

bool AudioChannelWorker::CanRead() const
{
	return IsOpen() && IsRecorder();
}

} // channels

} // audio

} // media

} // core
