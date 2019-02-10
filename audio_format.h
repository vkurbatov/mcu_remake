#ifndef AUDIO_FORMAT_H
#define AUDIO_FORMAT_H

#include <cstdint>

struct audio_format_t
{
    std::uint32_t   sample_rate;
    std::uint32_t   bit_per_sample;
    std::uint32_t   channels;

    audio_format_t(std::uint32_t sr = 0, std::uint32_t bps = 0, std::uint32_t c = 0)
        : sample_rate(sr)
        , bit_per_sample(bps)
        , channels(c)
    {}

    inline bool is_init() const { return sample_rate >= 8000 && bit_per_sample > 7 && channels > 0; }
    inline bool operator == (const audio_format_t& af) const { return sample_rate == af.sample_rate
                && bit_per_sample == af.bit_per_sample
                && channels == af.channels; }
    inline bool operator != (const audio_format_t& af) const { return !(af == *this); }
    inline std::uint32_t sample_octets() const { return (bit_per_sample * channels) / 8; }
    inline std::uint32_t bytes_per_second() const { return (sample_rate * bit_per_sample * channels) / 8; }
    inline std::uint32_t duration_ms(std::size_t size) const { return (size * 1000) / bytes_per_second(); }
    inline std::size_t octets_per_duration(std::uint32_t duration_ms) const { return (duration_ms * bytes_per_second()) / 1000; }
};


#endif // AUDIO_FORMAT_H
