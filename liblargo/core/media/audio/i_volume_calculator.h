#ifndef I_VOLUME_CALCULATOR_H
#define I_VOLUME_CALCULATOR_H

#include "audio_format.h"

namespace core
{

namespace media
{

namespace audio
{

class IVolumeCalculator
{
public:
	virtual ~IVolumeCalculator() { }

public:
	//IVolumeCalculator
	virtual double CalculateLevel(const void *data, std::size_t size, const audio_format_t &format) = 0;
};

} // audio

} // media

} // core

#endif // I_VOLUME_CALCULATOR_H
