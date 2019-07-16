#ifndef AUDIO_QUEUE_H
#define AUDIO_QUEUE_H

//#include "core/media/audio/i_audio_formatter.h"
#include "core/media/common/sync_point.h"
#include "core/media/common/data_queue.h"
#include "core/media/audio/i_audio_point.h"
#include "core/media/audio/audio_resampler.h"

namespace core
{

namespace media
{

namespace audio
{

const std::uint32_t audio_queue_option_peek = 1;

const std::uint32_t default_audio_queue_duration_ms = 1000;
const std::uint32_t default_audio_queue_jitter_ms = 1000;

class AudioQueue : public IAudioPoint, public IAudioFormatter, public IDataQueueControl, public ISyncPoint
{
	SyncPoint		m_sync_point;

	AudioResampler	m_resampler;
	DataQueue		m_data_queue;

	std::uint32_t	m_queue_duration_ms;
	std::uint32_t	m_jitter_ms;
	bool			m_can_read;

	media_buffer_t	m_input_resampler_buffer;
	media_buffer_t	m_output_resampler_buffer;

public:
	AudioQueue(const audio_format_t& audio_format
	           , std::uint32_t queue_duration_ms = default_audio_queue_duration_ms
	                   , std::uint32_t jitter_ms = default_audio_queue_jitter_ms
	                           , bool thread_safe = false);

private:
	bool check_jitter();

	// IAudioWriter interface
public:
	std::int32_t Write(const audio_format_t& audio_format, const void* data, std::size_t size, std::uint32_t options) override;

	// IAudioReader interface
public:
	std::int32_t Read(const audio_format_t& audio_format, void* data, std::size_t size, std::uint32_t options) override;

	// IAudioFormatter interface
public:
	const audio_format_t& GetAudioFormat() const override;
	bool SetAudioFormat(const audio_format_t& audio_format) override;

	// IDataQueueControl interface
public:
	void Reset() override;
	std::size_t Size() const override;
	std::size_t Capacity() const override;

	// ISyncPoint interface
public:
	void Lock() const override;
	void Unlock() const override;

	// IMediaWriteStatus interface
public:
	bool CanWrite() const override;

	// IMediaReadStatus interface
public:
	bool CanRead() const override;
};

} // audio

} // media

} // core

#endif // AUDIO_QUEUE_H
