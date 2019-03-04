#ifndef AUDIO_DIVIDER_H
#define AUDIO_DIVIDER_H

#include "media/audio/i_audio_point.h"
#include "media/audio/i_audio_formatter.h"

namespace core
{

namespace media
{

namespace audio
{

class AudioDivider: public IAudioWriter
{

	media_buffer_t		m_mix_buffer;

	IAudioWriter&		m_audio_writer1;
	IAudioWriter&		m_audio_writer2;


public:
	AudioDivider(IAudioWriter& audio_writer1, IAudioWriter& audio_writer2);

	// IAudioWriter interface
public:
	std::int32_t Write(const audio_format_t& audio_format, const void* data, std::size_t size, uint32_t options) override;


};

} // audio

} // media

} // core

#endif // AUDIO_DIVIDER_H
