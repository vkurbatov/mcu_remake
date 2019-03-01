#include "audio_stream.h"
namespace core
{

namespace media
{

namespace audio
{

AudioStream::AudioStream(media_stream_id_t stream_id
						 , const session_id_t &session_id
						 , const audio_format_t &audio_format
						 , IAudioPoint &audio_point
						 , bool is_writer)
	: m_stream_id(stream_id)
	, m_session_id(session_id)
	, m_audio_format(audio_format)
	, m_audio_point(audio_point)
	, m_is_writer(is_writer)
{

}

media_stream_id_t AudioStream::GetStreamId() const
{
	return m_stream_id;
}

const session_id_t& AudioStream::GetSessionId() const
{
	return m_session_id;
}

const audio_format_t& AudioStream::GetAudioFormat() const
{
	return m_audio_format;
}

bool AudioStream::SetAudioFormat(const audio_format_t& audio_format)
{
	m_audio_format = audio_format;

	return true;
}

std::int32_t AudioStream::Write(const void *data, std::size_t size, uint32_t options)
{
	return internal_write(m_audio_format, data, size, options);
}

std::int32_t AudioStream::Read(void *data, std::size_t size, uint32_t options)
{
	return internal_read(m_audio_format, data, size, options);
}

bool AudioStream::CanRead() const
{
	return !m_is_writer;
}

bool AudioStream::CanWrite() const
{
	return m_is_writer;
}

std::int32_t AudioStream::Write(const audio_format_t &audio_format, const void *data, std::size_t size, std::uint32_t options)
{
	return internal_write(audio_format, data, size, options);
}

std::int32_t AudioStream::Read(const audio_format_t &audio_format, void *data, std::size_t size, std::uint32_t options)
{
	return internal_read(audio_format, data, size, options);
}

std::int32_t AudioStream::internal_write(const audio_format_t &audio_format, const void* data, std::size_t size, std::uint32_t options)
{
	return CanWrite() ? m_audio_point.Write(audio_format, data, size, options) : -EACCES;
}

std::int32_t AudioStream::internal_read(const audio_format_t &audio_format, void* data, std::size_t size, std::uint32_t options)
{
	return CanRead() ? m_audio_point.Read(audio_format, data, size, options) :  -EACCES;
}


} // audio

} // media

} // core
