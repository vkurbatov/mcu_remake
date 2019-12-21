#ifndef VOLUME_CALCULATOR_H
#define VOLUME_CALCULATOR_H

#include "i_volume_calculator.h"
#include <cstdint>

namespace core
{

namespace media
{

namespace audio
{

class VolumeCalculator : public IVolumeCalculator
{
	double      m_max_level;
	double      m_current_level;

public:
	VolumeCalculator();
	virtual ~VolumeCalculator() { }

	//IVolumeCalculator
	virtual double CalculateLevel(const void *data, std::size_t size, const audio_format_t &format);
};

} // audio

} // media

} // core

#endif // VOLUME_CALCULATOR_H
