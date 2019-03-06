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
	static const std::uint32_t default_duration = 10;

	channel_direction_t		direction;
	audio_format_t			audio_format;
	std::uint32_t			buffer_duration_ms;
	bool					nonblock_mode;

	audio_channel_params_t(channel_direction_t dir = channel_direction_t::both
			, const audio_format_t& afmt = null_audio_format
			, std::uint32_t prd = 0
			, bool nonblock = false);

	bool is_valid() const;
	bool is_null() const;
	std::size_t buffer_size() const;
	bool is_recorder_only() const;
	bool is_playback_only() const;
	bool is_recorder() const;
	bool is_playback() const;

	bool operator == (const audio_channel_params_t& acp) const;
	bool operator != (const audio_channel_params_t& acp) const;
};


static const audio_channel_params_t default_audio_params = { channel_direction_t::both, default_audio_format, audio_channel_params_t::default_duration, false };
static const audio_channel_params_t null_audio_params = { channel_direction_t::none, null_audio_format, 0, false };

} // channels

} // audio

} // media

} // core



#endif
