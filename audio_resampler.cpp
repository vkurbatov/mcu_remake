#include "audio_resampler.h"

#include <limits>
#include <cstring>
#include <algorithm>

namespace reample_utils
{

template<typename Tin>
Tin clamp_channel_sample(const Tin* input_sample, std::size_t channels)
{
    Tin result = *input_sample;

    if (channels > 1)
    {

        double sample = 0;

        for (std::size_t in_idx = 1; in_idx < channels; in_idx++)
        {
            sample += static_cast<double>(input_sample[in_idx]);
        }

        result = static_cast<Tin>(sample / static_cast<double>(channels));
    }

    return result;
}

template<typename Tin,
         typename Tout,
         Tin Tin_max = std::numeric_limits<Tin>::max(),
         Tout Tout_max = std::numeric_limits<Tout>::max()>
std::size_t rescaling_sample(const Tin* input_sample, Tout* output_sample, std::size_t input_channels = 1, std::size_t output_channels = 1)
{

    for (std::size_t out_idx = 0; out_idx < output_channels; out_idx++)
    {
        std::size_t in_idx = (out_idx * input_channels) / output_channels;
        output_sample[out_idx] = static_cast<Tout>((static_cast<double>(clamp_channel_sample(input_sample + in_idx, input_channels / output_channels)) / static_cast<double>(Tin_max)) * static_cast<double>(Tout_max));
    }
    return sizeof(Tout) * output_channels;
}

template<typename Tin>
std::size_t rescaling_sample(const Tin* input_sample, void* output_sample, std::size_t output_sample_size, std::size_t input_channels = 1, std::size_t output_channels = 1)
{

    std::size_t result = 0;

    switch(output_sample_size)
    {
        case 8:
            result = rescaling_sample(input_sample, static_cast<std::int8_t*>(output_sample), input_channels, output_channels);
        break;
        case 16:
            result = rescaling_sample(input_sample, static_cast<std::int16_t*>(output_sample), input_channels, output_channels);
        break;
        case 32:
            result = rescaling_sample(input_sample, static_cast<std::int32_t*>(output_sample), input_channels, output_channels);
    }

    return result;
}

std::size_t rescaling_sample(const void* input_sample, std::size_t input_sample_size, void* output_sample, std::size_t output_sample_size, std::size_t input_channels = 1, std::size_t output_channels = 1)
{
    std::size_t result = 0;

    if (input_sample_size == output_sample_size)
    {
        result = input_sample_size / 8;
        std::memcpy(output_sample, input_sample, result);
    }
    else
    {
        switch(input_sample_size)
        {
            case 8:
                result = rescaling_sample(static_cast<const std::int8_t*>(input_sample), output_sample, output_sample_size, input_channels, output_channels);
            break;
            case 16:
                result = rescaling_sample(static_cast<const std::int16_t*>(input_sample), output_sample, output_sample_size, input_channels, output_channels);
            break;
            case 32:
                result = rescaling_sample(static_cast<const std::int32_t*>(input_sample), output_sample, output_sample_size, input_channels, output_channels);
        }
    }

    return result;
}

}

AudioResampler::AudioResampler()
{

}

std::int32_t AudioResampler::Resampling(const audio_format_t &input_format,
                           const audio_format_t &output_format,
                           const void *input_data, std::size_t input_size,
                           void *output_data, std::size_t output_size)
{
    std::int32_t result = -1;

    if (input_format.is_init() && output_format.is_init()
            && input_data != nullptr && output_data != nullptr
            && input_size > 0)
    {

        if (input_format != output_format)
        {

            // cut the sizes on both sides

            auto real_output_size = (input_size * output_format.bytes_per_second()) / input_format.bytes_per_second();

            if (output_size == 0 || output_size > real_output_size)
            {
                output_size = real_output_size;
            }
            else if (output_size < real_output_size)
            {
                input_size = (output_size * input_format.bytes_per_second()) / output_format.bytes_per_second();
            }

            auto input_ptr = static_cast<const std::uint8_t*>(input_data);
            auto output_ptr = static_cast<std::uint8_t*>(output_data);

            auto input_sample_size = input_format.sample_octets();
            auto output_sample_size = output_format.sample_octets();

            auto input_sample_count = input_size / input_sample_size;
            auto output_sample_count = output_size / output_sample_size;

            result = 0;

            for (auto out_idx = 0; out_idx < output_size; out_idx += output_sample_size)
            {
                auto in_idx = (out_idx * input_sample_count) / output_sample_count;

                in_idx -= in_idx % input_sample_size;

                result += reample_utils::rescaling_sample(
                            input_ptr + in_idx
                            , input_format.bit_per_sample
                            , output_ptr + out_idx
                            , output_format.bit_per_sample
                            , input_format.channels
                            , output_format.channels);
            }
        }
        else
        {
            output_size = input_size = std::min(input_size, output_size == 0 ? input_size : output_size);

            result = output_size;

            if (input_data != output_data)
            {
                std::memcpy(output_data, input_data, result);
            }
        }
    }

    return result;
}

int32_t AudioResampler::Resampling(const void *input_data, std::size_t input_size, void *output_data, std::size_t output_size)
{
    return Resampling(m_input_fromat, m_output_fromat, input_data, input_size, output_data, output_size);
}
