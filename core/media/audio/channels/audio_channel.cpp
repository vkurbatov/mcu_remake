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
{

}



bool AudioChannel::IsRecorder() const
{
	return GetAudioParams().direction == channel_direction_t::recorder || GetAudioParams().direction == channel_direction_t::both;
}

bool AudioChannel::IsPlayback() const
{
	return GetAudioParams().direction == channel_direction_t::playback || GetAudioParams().direction == channel_direction_t::both;
}

} // channels

} // audio

} // media

} // core
