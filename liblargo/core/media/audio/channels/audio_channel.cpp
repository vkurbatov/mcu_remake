#include "audio_channel.h"

namespace core
{

namespace media
{

namespace audio
{

namespace channels
{

AudioChannel::AudioChannel()
{

}

const audio_channel_params_t& AudioChannel::GetAudioParams() const
{
	return internal_get_audio_params();
}

bool AudioChannel::SetAudioParams(const audio_channel_params_t& audio_params)
{
	return internal_set_audio_params(audio_params);
}

bool AudioChannel::IsRecorder() const
{
	return GetAudioParams().is_recorder();
}

bool AudioChannel::IsPlayback() const
{
	return GetAudioParams().is_playback();
}

const audio_format_t &AudioChannel::GetAudioFormat() const
{
	return GetAudioParams().audio_format;
}

bool AudioChannel::SetAudioFormat(const audio_format_t &audio_format)
{
	auto audio_params = GetAudioParams();

	audio_params.audio_format = audio_format;

	return SetAudioParams(audio_params);
}

} // channels

} // audio

} // media

} // core
