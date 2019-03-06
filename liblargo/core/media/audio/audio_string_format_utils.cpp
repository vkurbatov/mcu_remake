#include "audio_string_format_utils.h"

std::ostream &operator<<(std::ostream &ostream, const core::media::audio::audio_format_t &audio_format)
{
	return ostream << audio_format.sample_rate << "Hz/"
				   << audio_format.sample_format << "/"
				   << audio_format.channels;
}

std::ostream &operator<<(std::ostream &ostream, const core::media::audio::audio_format_t::sample_format_t &sample_format)
{
	static const char* format_string[] = {
			"UNK",
			"PCM8",
			"PCM16",
			"PCM32",
			"FLT32",
			"FLT64"};

	return ostream << format_string[static_cast<std::int32_t>(sample_format)];
}

std::ostream &operator<<(std::ostream &ostream, const core::media::audio::channels::audio_channel_params_t &channel_params)
{
	return ostream << channel_params.direction << ", "
				   << channel_params.buffer_duration_ms << "ms, "
				   << (channel_params.nonblock_mode ? "nonblock" : "block") << ", ["
				   << channel_params.audio_format << "]";
}

std::ostream &operator<<(std::ostream &ostream, const core::media::audio::channels::channel_direction_t &direction)
{
	static const char* format_string[] = {
		"none",
		"recorder",
		"playback",
		"both" };

	return ostream << format_string[static_cast<std::int32_t>(direction)];
}

