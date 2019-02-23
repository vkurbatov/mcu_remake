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
	bool					m_is_writer;
	IAudioPoint&			m_audio_point;

private:
	AudioStream(media_stream_id_t stream_id
				, const session_id_t& session_id
				, const audio_format_t& audio_format
				, IAudioPoint& audio_point
				, bool is_writer = false);

	virtual ~AudioStream() override = default;

	// IMediaStream interface
public:
	media_stream_id_t GetStreamId() const override;

	// IAudioStream interface
public:
	const session_id_t& GetSessionId() const override;

	const audio_format_t& GetAudioFormat() const override;
	bool SetAudioFormat(const audio_format_t& audio_format) override;

	// IMediaPoint interface
public:
	std::int32_t Write(const void *data, std::size_t size, std::uint32_t options = 0) override;
	std::int32_t Read(void *data, std::size_t size, std::uint32_t options = 0) override;

	bool CanRead() const override;
	bool CanWrite() const override;

	// IAudioPoint interface
public:
	std::int32_t Write(const audio_format_t &audio_format, const void *data, std::size_t size, std::uint32_t options = 0) override;
	std::int32_t Read(const audio_format_t &audio_format, void *data, std::size_t size, std::uint32_t options = 0) override;

private:
	std::size_t internal_write(const audio_format_t &audio_format, const void* data, std::size_t size, std::uint32_t options = 0);
	std::size_t internal_read(const audio_format_t &audio_format, void* data, std::size_t size, std::uint32_t options = 0);
};

} // audio

} // media

} // core

#endif // AUDIO_STREAM_H
