#include "audio_channel.h"

namespace core
{

namespace media
{

namespace audio
{

namespace channels
{

AudioChannel::AudioChannel(const audio_channel_params_t& audio_params)
	: AudioPoint(audio_params.audio_format)
{

}

const audio_channel_params_t& AudioChannel::GetAudioParams() const
{
	return internal_get_audio_params();
}

bool AudioChannel::SetAudioParams(const audio_channel_params_t& audio_params)
{
	auto result = internal_set_audio_params(audio_params);

	if (result)
	{
		AudioPoint::SetAudioFormat(audio_params.audio_format);
	}

	return result;
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

	return SetAudioParams(audio_params) && AudioPoint::SetAudioFormat(audio_format);
}

} // channels

} // audio

} // media

} // core
