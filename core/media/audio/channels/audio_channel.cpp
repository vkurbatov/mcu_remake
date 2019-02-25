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
	: AudioPoint(audio_params.is_recorder() ? audio_params.audio_format : null_audio_format,
				 audio_params.is_playback() ? audio_params.audio_format : null_audio_format)
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
		if (IsRecorder())
		{
			AudioPoint::SetInputFormat(audio_params.audio_format);
		}

		if (IsPlayback())
		{
			AudioPoint::SetOutputFormat(audio_params.audio_format);
		}
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

const audio_format_t &AudioChannel::GetInputFormat() const
{
	return IsRecorder() ? GetAudioParams().audio_format : null_audio_format;
}

const audio_format_t &AudioChannel::GetOutputFormat() const
{
	return IsPlayback() ? GetAudioParams().audio_format : null_audio_format;
}

void AudioChannel::SetInputFormat(const audio_format_t &input_format)
{
	if (IsRecorder() == true)
	{
		auto audio_params = GetAudioParams();

		audio_params.audio_format = input_format;

		if (SetAudioParams(audio_params))
		{
			AudioPoint::SetInputFormat(input_format);
		}
	}
}

void AudioChannel::SetOutputFormat(const audio_format_t &output_format)
{
	if (IsPlayback() == true)
	{
		auto audio_params = GetAudioParams();

		audio_params.audio_format = output_format;

		if (SetAudioParams(audio_params))
		{
			AudioPoint::SetInputFormat(output_format);
		}
	}
}

} // channels

} // audio

} // media

} // core
