#include "audio_format.h"
#include "media/common/utils/format_converter.h"

namespace core
{

namespace media
{

namespace audio
{

bool audio_format_t::is_encoded(sample_format_t sample_format)
{
    return sample_format > sample_format_t::float_64;
}

bool audio_format_t::is_valid_sample_rate(int32_t sr)
{
	return sr >= min_sample_rate && sr <= max_sample_rate;
}

bool audio_format_t::is_valid_sample_format(sample_format_t sf)
{
	return sf >= sample_format_t::pcm_8 && sf <= sample_format_t::float_64;
}

bool audio_format_t::is_valid_channels(int32_t c)
{
	return c >= min_channels && c <= max_channels;
}

bool audio_format_t::is_float_format(sample_format_t sf)
{
	return sf == sample_format_t::float_32 || sf == sample_format_t::float_64;
}

bool audio_format_t::is_integer_format(sample_format_t sf)
{
    return sf == sample_format_t::pcm_8
            || sf == sample_format_t::pcm_16
            || sf == sample_format_t::pcm_32;
}

uint32_t audio_format_t::bit_per_sample(sample_format_t sf)
{
    static const std::uint32_t size_arr[] = { 0, 8, 16, 32, 32, 64, 0, 0, 0 };
	return size_arr[static_cast<std::int32_t>(sf)];
}

sample_format_t audio_format_t::format_from_bits(uint32_t bit_per_sample, bool integer_proirity)
{
	sample_format_t result = sample_format_t::unknown;

	switch (bit_per_sample)
	{
	case 8:
		result = sample_format_t::pcm_8;
    break;

	case 16:
		result = sample_format_t::pcm_16;
    break;

	case 32:
		result = integer_proirity ? sample_format_t::pcm_32 : sample_format_t::float_32;
    break;

	case 64:
		result = sample_format_t::float_64;
    break;
	}

	return result;
}

audio_format_t::audio_format_t(uint32_t sr, sample_format_t sf, uint32_t c)
	: sample_rate(sr)
	, sample_format(sf)
    , channels(c)
{

}

audio_format_t::audio_format_t(sample_format_t sample_format
                               , uint32_t sample_rate
                               , uint32_t channels)
    : sample_rate(sample_rate)
    , sample_format(sample_format)
    , channels(channels)
{

}

bool audio_format_t::is_valid() const
{
	return is_valid_sample_rate(sample_rate) && is_valid_sample_format(sample_format) && is_valid_channels(channels);
}

bool audio_format_t::is_null() const
{
	return sample_rate == 0 && sample_format == sample_format_t::unknown && channels == 0;
}

bool audio_format_t::is_float_format() const
{
	return is_float_format(sample_format);
}

bool audio_format_t::is_integer_format() const
{
	return is_integer_format(sample_format);
}

uint32_t audio_format_t::bit_per_sample() const
{
	return bit_per_sample(sample_format);
}

bool audio_format_t::operator == (const audio_format_t& af) const
{
	return sample_rate == af.sample_rate
	       && sample_format == af.sample_format
	       && channels == af.channels;
}

bool audio_format_t::operator != (const audio_format_t& af) const
{
	return !(af == *this);
}

size_t audio_format_t::bytes_per_sample() const
{
	return (bit_per_sample() * channels) / 8;
}

size_t audio_format_t::bytes_per_second() const
{
	return (sample_rate * bit_per_sample() * channels) / 8;
}

uint32_t audio_format_t::duration_ms(std::size_t size) const
{
	return is_valid() ? (size * 1000) / bytes_per_second() : 0;
}

std::size_t audio_format_t::size_from_duration(uint32_t duration_ms) const
{
	return (duration_ms * bytes_per_second()) / 1000;
}

std::size_t audio_format_t::size_from_format(const audio_format_t& af, std::size_t size) const
{
	return af.is_valid() ? size * bytes_per_second() / af.bytes_per_second() : 0;
}

size_t audio_format_t::samples_from_size(std::size_t size) const
{
	return is_valid() ? size / bytes_per_sample() : 0;
}

size_t audio_format_t::samples_from_duration(uint32_t duration_ms) const
{
    return samples_from_size(size_from_duration(duration_ms));
}

bool audio_format_t::is_encoded() const
{
    return is_encoded(sample_format);
}

bool audio_format_t::is_convertable() const
{
    return !is_encoded();
}

bool audio_format_t::is_planar() const
{
    return false;
}

std::size_t audio_format_t::frame_size() const
{
    return bytes_per_sample();
}

std::size_t audio_format_t::planes() const
{
    return 1;
}

plane_sizes_t audio_format_t::plane_sizes() const
{
    return { frame_size() };
}

std::string audio_format_t::to_string() const
{
    std::string format;
    format.append(utils::format_conversion::get_format_name(sample_format));
    format.append("/");
    format.append(std::to_string(sample_rate));
    format.append("/");
    format.append(std::to_string(channels));
    return format;
}

} // audio

} // media

} // core
