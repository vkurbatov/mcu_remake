#ifndef AUDIO_EVENT_H
#define AUDIO_EVENT_H

#include "media/common/i_media_point.h"
#include "media/common/delay_timer.h"
#include "media/audio/i_audio_point.h"
#include "media/audio/channels/file/file_channel.h"

namespace core
{

namespace media
{

namespace audio
{

namespace tools
{

class AudioEvent : public IAudioReader, public IMediaReadStatus
{

	DelayTimer						m_delay_timer;
	std::string						m_device_name;
	std::size_t						m_repetitions;
	std::uint32_t					m_delay_interval;
	std::size_t						m_current_repetition;

	channels::file::FileChannel		m_file;



public:

	AudioEvent(const std::string& device_name
			   , std::size_t repetitions
			   , std::size_t delay_interval);

	// IAudioReader interface
public:
	std::int32_t Read(const audio_format_t& audio_format, void* data, std::size_t size, std::uint32_t options = 0) override;

	// IMediaReadStatus interface
public:
	bool CanRead() const override;

public:
	void Reset();
	void Reset(const std::string& device_name
			   , std::size_t repetitions
			   , std::size_t delay_interval);
};

} // tools

} // audio

} // media

} // core

#endif // AUDIO_EVENT_H
