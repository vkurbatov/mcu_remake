#ifndef AUDIO_FORMAT_H
#define AUDIO_FORMAT_H

#include "core/media/common/media_types.h"
#include "core/media/common/media_format.h"
#include <ostream>

namespace core
{

namespace media
{

namespace audio
{

enum class sample_format_t
{
    unknown,
    pcm_8,
    pcm_16,
    pcm_32,
    float_32,
    float_64,
    pcma,
    pcmu,
    aac
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

struct audio_format_t : virtual public i_format_info
{

	std::uint32_t   sample_rate;
	sample_format_t sample_format;
	std::uint32_t   channels;


    static bool is_encoded(sample_format_t sapmpl_format);
	static bool is_valid_sample_rate(std::int32_t sr);
	static bool is_valid_sample_format(sample_format_t sf);
	static bool is_valid_channels(std::int32_t c);
	static bool is_float_format(sample_format_t sf);
	static bool is_integer_format(sample_format_t sf);
	static std::uint32_t bit_per_sample(sample_format_t sf);
	static sample_format_t format_from_bits(std::uint32_t bit_per_sample, bool integer_proirity = false);
    static sample_format_t raw_sample_format(sample_format_t sample_format);

	audio_format_t(std::uint32_t sr = 0, sample_format_t sf = sample_format_t::unknown, std::uint32_t c = 0);
    audio_format_t(sample_format_t sample_format, std::uint32_t sr = 0, std::uint32_t c = 0);

	bool is_valid() const;
	bool is_null() const;

	bool is_float_format() const;
	bool is_integer_format() const;

	std::uint32_t bit_per_sample() const;

	bool operator == (const audio_format_t& af) const;

	bool operator != (const audio_format_t& af) const;
	std::size_t bytes_per_sample() const;
	std::size_t bytes_per_second() const;
	std::uint32_t duration_ms(std::size_t size) const;
	std::size_t size_from_duration(std::uint32_t duration_ms) const;
	std::size_t size_from_format(const audio_format_t& af, std::size_t size) const;
	std::size_t samples_from_size(std::size_t size) const;
	std::size_t samples_from_duration(std::uint32_t duration_ms) const;
    sample_format_t raw_sample_format() const;

    // i_format_info interface
public:
    bool is_encoded() const override;
    bool is_convertable() const override;
    bool is_planar() const override;
    std::size_t frame_size() const override;
    std::size_t planes() const override;
    plane_sizes_t plane_sizes() const override;
    std::string to_string() const override;
};

static const audio_format_t default_audio_format = { default_sample_rate, sample_format_t::pcm_16, default_channels };
static const audio_format_t null_audio_format = { 0, sample_format_t::unknown, 0 };

} // audio

} // media

} // core


#endif // AUDIO_FORMAT_H
