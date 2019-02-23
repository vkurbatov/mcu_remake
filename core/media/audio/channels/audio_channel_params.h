#ifndef AUDIO_CHANNEL_PARAMS_H
#define AUDIO_CHANNEL_PARAMS_H

#include "media/audio/audio_format.h"

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
	channel_direction_t		direction;
	audio_format_t			audio_format;
	std::uint32_t			period;
	bool					nonblock_mode;

	audio_channel_params_t(channel_direction_t dir = channel_direction_t::both, const audio_format_t& afmt = null_audio_format, std::uint32_t prd = 0, bool nonblock = false)
		: direction(dir)
		, audio_format(afmt)
		, period(prd)
		, nonblock_mode(nonblock)
	{}

	inline bool is_valid() const { return direction != channel_direction_t::none && audio_format.is_valid(); }
	inline bool is_null() const { return direction == channel_direction_t::none && audio_format.is_null(); }
	inline std::size_t buffer_size() const { return audio_format.size_from_duration(period); }
	inline bool is_recorder_only() const { return direction == channel_direction_t::recorder; }
	inline bool is_playback_only() const { return direction == channel_direction_t::playback; }
	inline bool is_recorder() const { return direction == channel_direction_t::both || is_recorder_only(); }
	inline bool is_playback() const { return direction == channel_direction_t::both || is_recorder_only(); }

	bool operator == (const audio_channel_params_t& acp) { return direction == acp.direction
																	&& period == acp.period
																	&& nonblock_mode == acp.nonblock_mode
																	&& audio_format == acp.audio_format; }
	inline bool operator != (const audio_channel_params_t& acp) { return (*this == acp) == false; }
};


static const audio_channel_params_t default_audio_params = { channel_direction_t::both, default_audio_format, 0, false };
static const audio_channel_params_t null_audio_params = { channel_direction_t::none, null_audio_format, 10, false };

} // channels

} // audio

} // media

} // core



#endif
