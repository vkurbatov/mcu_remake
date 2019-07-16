#include "audio_channel_params.h"

namespace core
{

namespace media
{

namespace audio
{

namespace channels
{

audio_channel_params_t::audio_channel_params_t(channel_direction_t dir
        , const audio_format_t& afmt
        , uint32_t bt
        , std::uint32_t pt
        , std::uint32_t wt)
	: direction(dir)
	, audio_format(afmt)
	, buffer_time_ms(bt)
	, period_time_ms(pt)
	, wait_timeout_ms(wt)
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
	return audio_format.size_from_duration(buffer_time_ms);
}

std::size_t audio_channel_params_t::period_size() const
{
	return audio_format.size_from_duration(period_time_ms);
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

bool audio_channel_params_t::is_nonblock() const
{
	return wait_timeout_ms > 0;
}

bool audio_channel_params_t::operator ==(const audio_channel_params_t& acp) const
{
	return direction == acp.direction
	       && buffer_time_ms == acp.buffer_time_ms
	       && period_time_ms == acp.period_time_ms
	       && wait_timeout_ms == acp.wait_timeout_ms
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
