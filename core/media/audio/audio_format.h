#ifndef AUDIO_FORMAT_H
#define AUDIO_FORMAT_H

#include "media/common/media_types.h"

namespace core
{

namespace media
{

namespace audio
{

struct audio_format_t
{

    enum sample_format_t
    {
        unknown,
        pcm_8,
        pcm_16,
        pcm_32,
        float_32
    };

    static const std::uint32_t min_sample_rate = 8000;
    static const std::uint32_t max_sample_rate = 48000;

    static const std::uint32_t min_bit_per_sample = 8;
    static const std::uint32_t max_bit_per_sample = 32;

    static const std::uint32_t min_channels = 1;
    static const std::uint32_t max_channels = 2;

    std::uint32_t   sample_rate;
    std::uint32_t   bit_per_sample;
    std::uint32_t   channels;

    audio_format_t(std::uint32_t sr = 0, std::uint32_t bps = 0, std::uint32_t c = 0)
        : sample_rate(sr)
        , bit_per_sample(bps)
        , channels(c)
    {}

    static inline bool is_valid_sample_rate(std::int32_t sr) { return sr >= min_sample_rate && sr <= max_sample_rate; }
    static inline bool is_valid_bit_per_sample(std::int32_t bps) { return bps >= min_bit_per_sample && bps <= max_bit_per_sample; }
    static inline bool is_valid_channels(std::int32_t c) { return c >= min_channels && c <= max_channels; }
    // static sample_format_t sample_format(std::int32_t bps) { return switch (bps) { case }; }

    inline bool is_valid() const { return is_valid_sample_rate(sample_rate) && is_valid_bit_per_sample(bit_per_sample) && is_valid_channels(channels); }
    inline bool is_null() const { return sample_rate == 0 && bit_per_sample == 0 && channels == 0; }

    inline bool operator == (const audio_format_t& af) const { return sample_rate == af.sample_rate
                && bit_per_sample == af.bit_per_sample
                && channels == af.channels; }

    inline bool operator != (const audio_format_t& af) const { return !(af == *this); }
    inline std::uint32_t sample_octets() const { return (bit_per_sample * channels) / 8; }
    inline std::uint32_t bytes_per_second() const { return (sample_rate * bit_per_sample * channels) / 8; }
    inline std::uint32_t duration_ms(std::size_t size) const { return is_valid() ? (size * 1000) / bytes_per_second() : 0; }
	inline std::size_t octets_from_duration(std::uint32_t duration_ms) const { return (duration_ms * bytes_per_second()) / 1000; }
	inline std::size_t octets_from_format(const audio_format_t& af, std::size_t size) const { return is_valid() ? size * bytes_per_second() / af.bytes_per_second() : 0; }

};


static const audio_format_t default_audio_format = { 8000, 16, 1 };
static const audio_format_t null_audio_format = { 0, 0, 0 };

} // audio

} // media

} // core


#endif // AUDIO_FORMAT_H
