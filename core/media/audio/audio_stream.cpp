#include "audio_stream.h"
namespace core
{

namespace media
{

namespace audio
{

AudioStream::~AudioStream()
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
	return false;
}

std::size_t AudioStream::Push(const void* data, std::size_t size)
{
	return internal_push(data, size);
}

std::size_t AudioStream::Pop(void* data, std::size_t size)
{
	return internal_pop(data, size);
}

std::size_t AudioStream::Read(void* data, std::size_t size, bool from_tail) const
{
	return internal_read(data, size, from_tail);
}

std::size_t AudioStream::Drop(std::size_t size)
{
	return internal_drop(size);
}

void AudioStream::Reset()
{

}

std::size_t AudioStream::Size() const
{

}

std::size_t AudioStream::Capacity() const
{

}

std::size_t AudioStream::internal_push(const void* data, std::size_t size)
{

}

std::size_t AudioStream::internal_pop(void* data, std::size_t size)
{

}

std::size_t AudioStream::internal_read(void* data, std::size_t size, bool from_tail) const
{

}

std::size_t AudioStream::internal_drop(std::size_t size)
{

}

} // audio

} // media

} // core
