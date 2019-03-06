#include "audio_channel_params.h"

namespace core
{

namespace media
{

namespace audio
{

namespace channels
{

audio_channel_params_t::audio_channel_params_t(channel_direction_t dir, const audio_format_t& afmt, uint32_t prd, bool nonblock)
	: direction(dir)
	, audio_format(afmt)
	, buffer_duration_ms(prd)
	, nonblock_mode(nonblock)
{}

bool audio_channel_params_t::is_valid() const
{
	return direction != channel_direction_t::none && audio_format.is_valid();
}

bool audio_channel_params_t::is_null() const
{
	return direction == channel_direction_t::none && audio_format.is_null();
}

std::size_t audio_channel_params_t::buffer_size() const
{
	return audio_format.size_from_duration(buffer_duration_ms);
}

bool audio_channel_params_t::is_recorder_only() const
{
	return direction == channel_direction_t::recorder;
}

bool audio_channel_params_t::is_playback_only() const
{
	return direction == channel_direction_t::playback;
}

bool audio_channel_params_t::is_recorder() const
{
	return direction == channel_direction_t::both || is_recorder_only();
}

bool audio_channel_params_t::is_playback() const
{
	return direction == channel_direction_t::both || is_playback_only();
}

bool audio_channel_params_t::operator ==(const audio_channel_params_t& acp) const
{
	return direction == acp.direction
			&& buffer_duration_ms == acp.buffer_duration_ms
			&& nonblock_mode == acp.nonblock_mode
			&& audio_format == acp.audio_format;
}

bool audio_channel_params_t::operator !=(const audio_channel_params_t& acp) const
{
	return !(acp == *this);
}

} // channels

} // audio

} // media

} // core
