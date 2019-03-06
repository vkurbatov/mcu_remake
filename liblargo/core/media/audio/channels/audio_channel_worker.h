#ifndef AUDIO_CHANNEL_WORKER_H
#define AUDIO_CHANNEL_WORKER_H

#include "media/common/delay_timer.h"
#include "media/common/data_queue.h"
#include "media/common/sync_point.h"
#include "media/audio/channels/audio_channel.h"


#include <thread>
#include <atomic>

namespace core
{

namespace media
{

namespace audio
{

namespace channels
{

class AudioChannelWorker : public AudioChannel
{
	SyncPoint			m_sync_point;

	IAudoChannel&		m_audio_channel;
	IMediaPoint&		m_media_point;

	std::thread			m_dispatch_thread;
	std::atomic_bool	m_running;

	DataQueue			m_audio_queue;

public:
	AudioChannelWorker(IAudoChannel& audio_channel, IMediaPoint& media_point, std::size_t queue_size);
	~AudioChannelWorker() override;

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

	// IMediaWriteStatus interface
public:
	bool CanWrite() const override;

	// IMediaReadStatus interface
public:
	bool CanRead() const override;
};

} // channels

} // audio

} // media

} // core

#endif // AUDIO_DISPATCHER_H
