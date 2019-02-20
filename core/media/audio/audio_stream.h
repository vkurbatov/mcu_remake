#ifndef AUDIO_STREAM_H
#define AUDIO_STREAM_H

#include "media/audio/i_audio_stream.h"
#include "media/audio/i_audio_point.h"
#include "media/audio/audio_format.h"
#include <unordered_map>

namespace core
{

namespace media
{

namespace audio
{

class AudioStream : public IAudioStream
{

	friend class AudioServer;

	media_stream_id_t		m_stream_id;
	session_id_t			m_session_id;
	audio_format_t			m_audio_format;
	bool					m_is_reader;
	IAudioPoint&			m_source_point;

private:
	AudioStream(media_stream_id_t  stream_id, const session_id_t& session_id, const audio_format_t& audio_format, IAudioPoint& audio_point);
	virtual ~AudioStream() override;

	// IMediaStream interface
public:
	media_stream_id_t GetStreamId() const override;

	// IAudioStream interface
public:
	const session_id_t& GetSessionId() const;

	const audio_format_t& GetAudioFormat() const override;
	bool SetAudioFormat(const audio_format_t& audio_format) override;

	// IDataQueueWriter interface
public:
	std::size_t Push(const void* data, std::size_t size) override;

	// IDataQueueReader interface
public:
	std::size_t Pop(void* data, std::size_t size) override;
	std::size_t Read(void* data, std::size_t size, bool from_tail) const override;
	std::size_t Drop(std::size_t size) override;

	// IDataQueueControl interface
public:
	void Reset() override;
	std::size_t Size() const override;
	std::size_t Capacity() const override;

private:
	std::size_t internal_push(const void* data, std::size_t size);
	std::size_t internal_pop(void* data, std::size_t size);
	std::size_t internal_read(void* data, std::size_t size, bool from_tail = false) const;
	std::size_t internal_drop(std::size_t size);
};

} // audio

} // media

} // core

#endif // AUDIO_STREAM_H
