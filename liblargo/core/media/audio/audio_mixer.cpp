#include "audio_mixer.h"
#include <cstring>

#include <limits>

#include <core-tools/logging.h>
#include "core/media/audio/audio_string_format_utils.h"

#define PTraceModule() "audio_mixer"

#define SCALED_MIXING	0

namespace core
{

namespace media
{

namespace audio
{

namespace audio_mixer_utils
{

enum mix_method_t
{
	mix,
	demix
};

/*
 * TODO: need impulse limits clamp
template<typename T>
T max_val() { return std::numeric_limits<T>::max(); }

template<> float max_val() { return 1.0f; }
template<> double max_val() { return 1.0; }
*/

template<typename T>
inline const T& clamp_value(const T& v1, const T& v2)
{
	return v2 == 0 ? v1 : std::min(v1, v2);
}

template<typename T>
inline const T scale_sample(const T& sample, std::size_t sample_count, mix_method_t mix_method)
{
#if	SCALED_MIXING
	return sample_count < 2
			? sample
			: static_cast<T>(mix_method == mix_method_t::mix
				? static_cast<double>(sample) / static_cast<double>(sample_count)
				: static_cast<double>(sample) * static_cast<double>(sample_count) / static_cast<double>(sample_count - 1));
#else
	return sample;
#endif
}

template<typename T>
inline void mix_sample(const T& input_sample, const T& mixed_sample, T& output_sample, std::size_t sample_count, mix_method_t mix_method)
{
	auto sample = scale_sample(input_sample, sample_count, mix_method_t::mix);

	if (mix_method == mix_method_t::mix)
	{
		output_sample = mixed_sample + sample;
	}
	else
	{
		output_sample = scale_sample(mixed_sample - sample, sample_count, mix_method_t::demix);
	}
}

template <typename T>
std::size_t mixed(std::size_t stream_count, mix_method_t mix_method,
				  const void* input_data, std::size_t input_data_size,
				  const void* mixed_data, std::size_t mixed_data_size,
				  void* output_data, std::size_t output_data_size = 0)
{
	auto input_samples = static_cast<const T*>(input_data);
	auto mixed_samples = static_cast<const T*>(mixed_data);
	auto output_samples = static_cast<T*>(output_data);

	output_data_size = clamp_value(input_data_size, output_data_size);
	input_data_size = std::min(input_data_size, output_data_size);
	mixed_data_size = std::min(mixed_data_size, output_data_size);

	auto sample_mixed_count = mixed_data_size / sizeof(T);
	auto sample_output_count = output_data_size / sizeof(T);

	if (stream_count < 1)
	{		
		std::memcpy(output_data, input_data, output_data_size);
	}
	else
	{
		std::size_t i = 0;

		for (i = 0; i < sample_mixed_count; i++)
		{
			mix_sample(input_samples[i], mixed_samples[i], output_samples[i], stream_count, mix_method);
		}

		for (; i < sample_output_count; i++)
		{
			mix_sample(input_samples[i], static_cast<T>(0), output_samples[i], stream_count, mix_method);
		}

	}

	return output_data_size;
}

std::size_t mixed(const audio_format_t& audio_format, std::size_t stream_count, mix_method_t mix_method,
				  const void* input_data, std::size_t input_data_size,
				  const void* mixed_data, std::size_t mixed_data_size,
				  void* output_data, std::size_t output_data_size = 0)
{
	size_t result = 0;

	switch(audio_format.sample_format)
	{
		case audio_format_t::sample_format_t::pcm_8:
			result = mixed<std::int8_t>(stream_count, mix_method, input_data, input_data_size, mixed_data, mixed_data_size, output_data, output_data_size);
			break;
		case audio_format_t::sample_format_t::pcm_16:
			result = mixed<std::int16_t>(stream_count, mix_method, input_data, input_data_size, mixed_data, mixed_data_size, output_data, output_data_size);
			break;
		case audio_format_t::sample_format_t::pcm_32:
			result = mixed<std::int32_t>(stream_count, mix_method, input_data, input_data_size, mixed_data, mixed_data_size, output_data, output_data_size);
			break;
		case audio_format_t::sample_format_t::float_32:
			result = mixed<float>(stream_count, mix_method, input_data, input_data_size, mixed_data, mixed_data_size, output_data, output_data_size);
			break;
		case audio_format_t::sample_format_t::float_64:
			result = mixed<double>(stream_count, mix_method, input_data, input_data_size, mixed_data, mixed_data_size, output_data, output_data_size);
			break;
		default:
			// ??
			break;

	}

	return result;
}

} // audio_mixer_utils

//------------------------------------------------------------------------------------------------

std::size_t AudioMixer::Mixed(const audio_format_t& audio_format,
							  std::size_t stream_count,
							  const void* input_data, std::size_t input_data_size,
							  const void* mixed_data, std::size_t mixed_data_size,
							  void* output_data, std::size_t output_data_size)
{

	return audio_mixer_utils::mixed(audio_format, stream_count,
									audio_mixer_utils::mix_method_t::mix,
									input_data, input_data_size,
									mixed_data, mixed_data_size,
									output_data, output_data_size);
}

std::size_t AudioMixer::Mixed(const audio_format_t& audio_format, std::size_t stream_count,
							  const void* input_data, std::size_t input_data_size,
							  void* mixed_data, std::size_t mixed_data_size)
{
	return Mixed(audio_format, stream_count,
				 input_data, input_data_size,
				 mixed_data, mixed_data_size,
				 mixed_data, mixed_data_size);
}

std::size_t AudioMixer::Demixed(const audio_format_t& audio_format, std::size_t stream_count,
								const void* input_data, std::size_t input_data_size,
								const void* mixed_data, std::size_t mixed_data_size,
								void* output_data, std::size_t output_data_size)
{
	return audio_mixer_utils::mixed(audio_format, stream_count,
									audio_mixer_utils::mix_method_t::demix,
									input_data, input_data_size,
									mixed_data, mixed_data_size,
									output_data, output_data_size);
}

std::size_t AudioMixer::Demixed(const audio_format_t& audio_format, std::size_t stream_count,
								const void* input_data, std::size_t input_data_size,
								void* mixed_data, std::size_t mixed_data_size)
{
	return Demixed(audio_format, stream_count,
				 input_data, input_data_size,
				 mixed_data, mixed_data_size,
				 mixed_data, mixed_data_size);
}

//------------------------------------------------------------------------------------------------

AudioMixer::AudioMixer(const audio_format_t& audio_format, bool is_mixed_mode)
	: m_audio_format(audio_format)
	, m_is_mixed_mode(is_mixed_mode)
{

}

std::size_t AudioMixer::operator()(std::size_t stream_count,
								   const void* input_data, std::size_t input_data_size,
								   const void* mixed_data, std::size_t mixed_data_size,
								   void* output_data, std::size_t output_data_size)
{
	return m_is_mixed_mode
			? Mixed(m_audio_format, stream_count, input_data, input_data_size, mixed_data, mixed_data_size, output_data, output_data_size)
			: Demixed(m_audio_format, stream_count, input_data, input_data_size, mixed_data, mixed_data_size, output_data, output_data_size);
}

std::size_t AudioMixer::operator()(std::size_t stream_count, const void* input_data, std::size_t input_data_size, void* mixed_data, std::size_t mixed_data_size)
{
	return operator()(stream_count, input_data, input_data_size, mixed_data, mixed_data_size, mixed_data, mixed_data_size);
}

const audio_format_t& AudioMixer::GetAudioFormat() const
{
	return m_audio_format;
}

bool AudioMixer::SetAudioFormat(const audio_format_t& audio_format)
{
	m_audio_format = audio_format;

	return true;
}

void AudioMixer::SetMixedMode(bool is_mixed_mode)
{
	m_is_mixed_mode = is_mixed_mode;
}

bool AudioMixer::IsMixedMode() const
{
	return m_is_mixed_mode;
}

} // audio

} // media

} // core
