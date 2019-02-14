#include "audio_resampler.h"

#include <limits>
#include <cstring>
#include <algorithm>

namespace core
{

namespace media
{

namespace audio
{

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

std::size_t rescaling_sample(const void* input_sample
							 , std::size_t input_sample_size
							 , void* output_sample
							 , std::size_t output_sample_size
							 , std::size_t input_channels = 1
							 , std::size_t output_channels = 1)
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

} // resample utils

// ----------------------------------------------------------------------------------------------------------

std::int32_t AudioResampler::Resampling(
		const audio_format_t &input_format
		, const audio_format_t &output_format
		, const void *input_data, std::size_t input_size
		, void *output_data, std::size_t output_size)
{
	std::int32_t result = -1;

	if (input_format.is_valid() && output_format.is_valid()
			&& input_data != nullptr && output_data != nullptr
			&& input_size > 0)
	{

		if (input_format != output_format)
		{

			// cut the sizes on both sides

			auto real_output_size = output_format.octets_from_format(input_format, input_size);

			if ((output_size == 0) || (output_size > real_output_size))
			{
				output_size = real_output_size;
			}
			else if (output_size < real_output_size)
			{
				input_size = (output_size * input_format.bytes_per_second()) / output_format.bytes_per_second();
			}

			auto input_sample_count = input_size / input_format.sample_octets();
			auto output_sample_count = output_size / output_format.sample_octets();

			result = 0;

			for (auto out_idx = 0; out_idx < output_size; out_idx += output_format.sample_octets())
			{

				auto in_idx = (out_idx * input_sample_count) / output_sample_count;

				in_idx -= in_idx % input_format.sample_octets();

				result += reample_utils::rescaling_sample(
							static_cast<const std::uint8_t*>(input_data) + in_idx
							, input_format.bit_per_sample
							, static_cast<std::uint8_t*>(output_data) + out_idx
							, output_format.bit_per_sample
							, input_format.channels
							, output_format.channels);
			}
		}
		else
		{
			output_size = input_size = std::min(input_size, output_size == 0 ? input_size : output_size);

			result = output_size;

			if ((result > 0) && (input_data != output_data))
			{
				std::memcpy(output_data, input_data, result);
			}
		}

	}

	return result;
}

int32_t AudioResampler::Resampling(
		const audio_format_t& input_format
		, const audio_format_t& output_format
		, const void* input_data
		, std::size_t input_size
		, audio_buffer_t& output_buffer)
{
	auto output_size = output_format.octets_from_format(input_format, input_size);

	if (output_buffer.size() != output_size)
	{
		output_buffer.resize(output_size);
		// LOG ???
	}

	return Resampling(input_format, output_format, input_data, input_size, output_buffer.data(), output_buffer.size());
}

int32_t AudioResampler::Resampling(const audio_format_t& input_format, const audio_format_t& output_format, const audio_buffer_t& input_buffer, audio_buffer_t& output_buffer)
{
	return Resampling(input_format, output_format, input_buffer.data(), input_buffer.size(), output_buffer);
}

int32_t AudioResampler::Resampling(const audio_format_t& input_format, const audio_format_t& output_format, const audio_buffer_t& input_buffer, void* output_data, std::size_t output_size)
{
	return Resampling(input_format, output_format, input_buffer.data(), input_buffer.size(), output_data, output_size);
}

// ----------------------------------------------------------------------------------------------------------

AudioResampler::AudioResampler(const audio_format_t& input_format, const audio_format_t& output_format)
	: m_input_format(input_format)
	, m_output_format(output_format)
{

}

int32_t AudioResampler::operator()(const void *input_data, std::size_t input_size, void *output_data, std::size_t output_size) const
{
	return Resampling(GetInputFormat(), GetOutputFormat(), input_data, input_size, output_data, output_size);
}

int32_t AudioResampler::operator()(const void *input_data, std::size_t input_size, const audio_format_t &input_format, void *output_data, std::size_t output_size) const
{
	return Resampling(input_format, GetOutputFormat(), input_data, input_size, output_data, output_size);
}

int32_t AudioResampler::operator()(const audio_buffer_t& input_buff, void* output_data, std::size_t output_size) const
{
	return Resampling(GetInputFormat(), GetOutputFormat(), input_buff, output_data, output_size);
}

int32_t AudioResampler::operator()(const audio_buffer_t& input_buff, const audio_format_t& input_format, void* output_data, std::size_t output_size) const
{
	return Resampling(input_format, GetOutputFormat(), input_buff, output_data, output_size);
}

const audio_buffer_t& AudioResampler::operator()(const void* input_data, std::size_t input_size, const audio_format_t& input_format)
{
	Resampling(input_format, GetOutputFormat(), input_data, input_size, m_internal_resampler_buffer);

	return m_internal_resampler_buffer;
}

const audio_buffer_t& AudioResampler::operator()(const audio_buffer_t& input_buff, const audio_format_t& input_format)
{
	Resampling(input_format, GetOutputFormat(), input_buff.data(), input_buff.size(), m_internal_resampler_buffer);

	return m_internal_resampler_buffer;
}

const audio_buffer_t& AudioResampler::operator()(const void* input_data, std::size_t input_size)
{
	operator()(input_data, input_size, GetInputFormat());

	return m_internal_resampler_buffer;
}

const audio_buffer_t& AudioResampler::operator()(const audio_buffer_t& input_buff)
{
	operator()(input_buff, GetInputFormat());

	return m_internal_resampler_buffer;
}

const audio_format_t& AudioResampler::GetInputFormat() const
{
	return m_input_format;
}

const audio_format_t& AudioResampler::GetOutputFormat() const
{
	return m_output_format;
}

void AudioResampler::SetInputFormat(const audio_format_t& input_format)
{
	m_input_format = input_format;
}

void AudioResampler::SetOutputFormat(const audio_format_t& output_format)
{
	m_output_format = output_format;
}

} // audio

} // media

} // core
