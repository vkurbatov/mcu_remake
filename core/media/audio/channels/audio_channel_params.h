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
	std::uint32_t			buffer_size;
	bool					nonblock_mode;

	audio_channel_params_t(channel_direction_t dir = channel_direction_t::both, const audio_format_t& afmt = null_audio_format, std::uint32_t bsz = 0, bool nonblock = false)
		: direction(dir)
		, audio_format(afmt)
		, buffer_size(bsz)
		, nonblock_mode(nonblock)
	{}

    inline bool is_init() const { return audio_format.is_valid(); }
};


static const audio_channel_params_t default_audio_params = { channel_direction_t::both, default_audio_format, 0, false };
static const audio_channel_params_t null_audio_params = { channel_direction_t::none, null_audio_format, 0, false };

} // channels

} // audio

} // media

} // core



#endif
