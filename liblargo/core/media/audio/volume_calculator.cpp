#include "volume_calculator.h"
#include <cmath>

namespace core
{

namespace media
{

namespace audio
{

template <class T>
double level_calculate(const void *data, std::size_t size, const audio_format_t &format)
{
	double accum_value = 0.0;
	size_t actual_size = size / format.bytes_per_sample();

	for (int i = 0; i < actual_size; ++i)
	{
		accum_value += std::fabs(static_cast<const T *>(data)[i]);
	}

	return accum_value / actual_size;
}

VolumeCalculator::VolumeCalculator()
	: m_max_level(1000.0)
	, m_current_level(0.0)
{

}

double VolumeCalculator::CalculateLevel(const void *data, std::size_t size, const audio_format_t &format)
{
	double result = 0.0;

	switch (format.sample_format)
	{
	case audio_format_t::sample_format_t::pcm_8:
		result = level_calculate<std::int8_t>(data, size, format);
		break;

	case audio_format_t::sample_format_t::pcm_16:
		result = level_calculate<std::int16_t>(data, size, format);
		break;

	case audio_format_t::sample_format_t::pcm_32:
		result = level_calculate<std::int32_t>(data, size, format);
		break;
	case audio_format_t::sample_format_t::float_32:
		result = level_calculate<float>(data, size, format);
		break;

	case audio_format_t::sample_format_t::float_64:
		result = level_calculate<double>(data, size, format);
		break;
	}

	m_current_level += (result - m_current_level) * 0.5;

	if (m_current_level > m_max_level)
	{
		m_max_level = m_current_level;
	}

	return (m_current_level / m_max_level) * 100.0;
}

} // audio

} // media

} // core
