#include "audio_channel_worker.h"
#include "media/common/timer.h"

namespace core
{

namespace media
{

namespace audio
{

namespace channels
{

AudioChannelWorker::AudioChannelWorker(IAudoChannel& audio_channel, IMediaPoint& media_point, std::size_t queue_size)
	: AudioChannel(audio_channel.GetAudioParams())
	, m_audio_channel(audio_channel)
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
		m_running = true;
		m_dispatch_thread = std::thread(&AudioChannelWorker::audio_dispatcher_proc, this, device_name);
	}

	return m_running;
}

bool AudioChannelWorker::Close()
{
	bool result = false;

	if (m_running)
	{
		m_running = false;

		result = m_dispatch_thread.joinable();

		if (result)
		{
			m_dispatch_thread.join();
		}
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
	lock_t lock(m_mutex);
	return m_audio_queue.Push(data, size);
}

int32_t AudioChannelWorker::internal_read(void* data, std::size_t size, uint32_t options)
{
	lock_t lock(m_mutex);
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
			lock_t lock(m_mutex);

			m_audio_queue.Reset();
		}

		result = m_audio_channel.SetAudioParams(audio_params);
	}

	return result;
}

void AudioChannelWorker::audio_dispatcher_proc(const std::string device_name)
{

	Timer timer;

	media_buffer_t buffer( m_audio_channel.GetAudioParams().buffer_size());

	auto delay_ms = m_audio_channel.GetAudioParams().duration;

	if (m_audio_channel.Open(device_name))
	{
		while (m_running)
		{

			if (IsPlayback())
			{
				std::size_t result = 0;
				{
					lock_t lock(m_mutex);
					result = m_audio_queue.Pop(buffer.data(), buffer.size());
				}

				if (result > 0)
				{
					result = m_media_point.Write(buffer.data(), result);
				}
			}

			if (IsRecorder())
			{
				std::size_t result = m_media_point.Read(buffer.data(), buffer.size());

				if (result > 0)
				{
					lock_t lock(m_mutex);
					result = m_audio_queue.Push(buffer.data(), result);
				}
			}

			timer(delay_ms);
		}

		m_audio_channel.Close();
	}
}

} // channels

} // audio

} // media

} // core
