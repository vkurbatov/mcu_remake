#ifndef AUDIO_FORMAT_H
#define AUDIO_FORMAT_H

#include "media/common/media_types.h"

namespace core
{

namespace media
{

namespace audio
{

/*static const sample_format_t min_sample_format = sample_format_t::pcm_8;
static const sample_format_t max_sample_format = sample_format_t::float_64;
static const sample_format_t default_sample_format = sample_format_t::pcm_16;*/

struct audio_format_t
{

	enum class sample_format_t
	{
		unknown,
		pcm_8,
		pcm_16,
		pcm_32,
		float_32,
		float_64
	};

	static const std::uint32_t min_sample_rate = 8000;
	static const std::uint32_t max_sample_rate = 48000;
	static const std::uint32_t default_sample_rate = 8000;

	static const std::uint32_t min_bit_per_sample = 8;
	static const std::uint32_t max_bit_per_sample = 32;
	static const std::uint32_t default_bit_per_sample = 16;

	static const std::uint32_t min_channels = 1;
	static const std::uint32_t max_channels = 2;
	static const std::uint32_t default_channels = 1;

	std::uint32_t   sample_rate;
	sample_format_t sample_format;
	std::uint32_t   channels;

	audio_format_t(std::uint32_t sr = 0, sample_format_t sf = sample_format_t::unknown, std::uint32_t c = 0)
		: sample_rate(sr)
		, sample_format(sf)
		, channels(c)
	{}

	static inline bool is_valid_sample_rate(std::int32_t sr) { return sr >= min_sample_rate && sr <= max_sample_rate; }
	static inline bool is_valid_sample_format(sample_format_t sf) { return sf >= sample_format_t::pcm_8 && sf <= sample_format_t::float_64; }
	static inline bool is_valid_channels(std::int32_t c) { return c >= min_channels && c <= max_channels; }
	static inline std::uint32_t bit_per_sample(sample_format_t sf)
	{
		static const std::uint32_t size_arr[] = { 0, 8, 16, 32, 32, 64 };
		return size_arr[static_cast<std::int32_t>(sf)];
	}

	static sample_format_t format_from_bits(std::uint32_t bit_per_sample)
	{
		sample_format_t result = sample_format_t::unknown;

		switch(bit_per_sample)
		{
			case 8:
				result = sample_format_t::pcm_8;
				break;
			case 16:
				result = sample_format_t::pcm_16;
				break;
			case 32:
				result = sample_format_t::float_32;
				break;
			case 64:
				result = sample_format_t::float_64;
		}

		return result;
	}

	// static sample_format_t sample_format(std::int32_t bps) { return switch (bps) { case }; }

	inline bool is_valid() const { return is_valid_sample_rate(sample_rate) && is_valid_sample_format(sample_format) && is_valid_channels(channels); }
	inline bool is_null() const { return sample_rate == 0 && sample_format == sample_format_t::unknown && channels == 0; }
	inline std::uint32_t bit_per_sample() const { return bit_per_sample(sample_format); }

	inline bool operator == (const audio_format_t& af) const { return sample_rate == af.sample_rate
				&& sample_format == af.sample_format
				&& channels == af.channels; }

	inline bool operator != (const audio_format_t& af) const { return !(af == *this); }
	inline std::uint32_t bytes_per_sample() const { return (bit_per_sample() * channels) / 8; }
	inline std::uint32_t bytes_per_second() const { return (sample_rate * bit_per_sample() * channels) / 8; }
	inline std::uint32_t duration_ms(std::size_t size) const { return is_valid() ? (size * 1000) / bytes_per_second() : 0; }
	inline std::size_t size_from_duration(std::uint32_t duration_ms) const { return (duration_ms * bytes_per_second()) / 1000; }
	inline std::size_t size_from_format(const audio_format_t& af, std::size_t size) const { return is_valid() ? size * bytes_per_second() / af.bytes_per_second() : 0; }
	inline std::uint32_t samples_from_size(std::size_t size) const { return is_valid() ? size / bytes_per_sample() : 0; }

};

static const audio_format_t default_audio_format = { audio_format_t::default_sample_rate, audio_format_t::sample_format_t::pcm_16, audio_format_t::default_channels };
static const audio_format_t null_audio_format = { 0, audio_format_t::sample_format_t::unknown, 0 };

} // audio

} // media

} // core


#endif // AUDIO_FORMAT_H
