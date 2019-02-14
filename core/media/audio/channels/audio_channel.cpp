#include "audio_channel.h"

namespace core
{

namespace media
{

namespace audio
{

namespace channels
{

AudioChannel::AudioChannel(const audio_channel_params_t& param)
	: AudioPoint(param.audio_format, param.audio_format)
	, m_mute(false)
	, m_volume(max_volume)
{

}

int32_t AudioChannel::Open(const std::string& device_name)
{

}

int32_t AudioChannel::Close()
{

}

bool AudioChannel::IsOpen() const
{

}

const audio_channel_params_t&AudioChannel::GetAudioParams() const
{

}

bool AudioChannel::SetAudioParams(const audio_channel_params_t& audio_params)
{

}

bool AudioChannel::IsReader() const
{

}

bool AudioChannel::IsPlayback() const
{

}

uint32_t AudioChannel::GetVolume() const
{

}

void AudioChannel::SetVolume(uint32_t volume)
{

}

bool AudioChannel::GetMute() const
{

}

void AudioChannel::SetMute(bool mute)
{

}

const std::string&AudioChannel::GetName() const
{

}

} // channels

} // audio

} // media

} // core
