#ifndef AUDIO_DISPATCHER_H
#define AUDIO_DISPATCHER_H

#include "media/audio/channels/i_audio_dispatcher.h"
#include "media/common/timer.h"
#include "media/common/data_queue.h"
#include "media/audio/channels/audio_channel.h"

#include <thread>
#include <mutex>
#include <atomic>

namespace core
{

namespace media
{

namespace audio
{

namespace channels
{

class AudioDispatcher : public AudioChannel
{
	using mutex_t = std::mutex;
	using lock_t = std::lock_guard<mutex_t>;

	IAudoChannel&		m_audio_channel;
	IMediaPoint&		m_media_point;

	std::thread			m_dispatch_thread;
	mutex_t				m_mutex;
	std::atomic_bool	m_running;

	DataQueue			m_audio_queue;

public:
	AudioDispatcher(IAudoChannel& audio_channel, IMediaPoint& media_point, std::size_t queue_size);
	~AudioDispatcher() override;

	// IAudoChannel interface
public:
	bool Open(const std::string& device_name) override;
	bool Close() override;
	bool IsOpen() const override;
	bool IsRecorder() const override;
	bool IsPlayback() const override;
	const std::string& GetName() const override;

	// MediaPoint interface
protected:
	int32_t internal_write(const void* data, std::size_t size, uint32_t options = 0) override;
	int32_t internal_read(void* data, std::size_t size, uint32_t options = 0) override;

	// AudioChannel interface
protected:
	const audio_channel_params_t&internal_get_audio_params() const override;
	bool internal_set_audio_params(const audio_channel_params_t& audio_params) override;

private:
	void audio_dispatcher_proc(const std::string device_name);
};

} // channels

} // audio

} // media

} // core

#endif // AUDIO_DISPATCHER_H
