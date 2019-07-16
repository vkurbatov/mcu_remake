#ifndef AUDIO_CHANNEL_PARAMS_H
#define AUDIO_CHANNEL_PARAMS_H

#include "core/media/audio/audio_format.h"

namespace core
{

namespace media
{

namespace audio
{

namespace channels
{

enum channel_direction_t
{
	none,
	recorder,
	playback,
	both
};

struct audio_channel_params_t
{
	static const std::uint32_t default_buffer_time = 30;
	static const std::uint32_t default_period_time = 10;
	static const std::uint32_t default_wait_timeout = 0;

	channel_direction_t		direction;
	audio_format_t			audio_format;
	std::uint32_t			buffer_time_ms;
	std::uint32_t           period_time_ms;
	std::uint32_t			wait_timeout_ms;

	audio_channel_params_t(channel_direction_t dir = channel_direction_t::both
	                       , const audio_format_t& afmt = null_audio_format
	                               , std::uint32_t bt = 0
	                                       , std::uint32_t pt = 0
	                                               , std::uint32_t wt = default_wait_timeout);

	bool is_valid() const;
	bool is_null() const;
	std::size_t buffer_size() const;
	std::size_t period_size() const;
	bool is_recorder_only() const;
	bool is_playback_only() const;
	bool is_recorder() const;
	bool is_playback() const;
	bool is_nonblock() const;

	bool operator == (const audio_channel_params_t& acp) const;
	bool operator != (const audio_channel_params_t& acp) const;
};


static const audio_channel_params_t default_audio_params = { channel_direction_t::both, default_audio_format, audio_channel_params_t::default_buffer_time, audio_channel_params_t::default_period_time, audio_channel_params_t::default_wait_timeout };
static const audio_channel_params_t null_audio_params = { channel_direction_t::none, null_audio_format, 0, 0, 0 };

} // channels

} // audio

} // media

} // core



#endif
