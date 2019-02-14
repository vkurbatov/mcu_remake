#ifndef AUDIO_FORMATTER_H
#define AUDIO_FORMATTER_H

#include "audio_format.h"

namespace core
{

namespace media
{

namespace audio
{

class IAudioFormatter
{

public:

	virtual ~IAudioFormatter() = default;

	virtual const audio_format_t& GetInputFormat() const = 0;
	virtual const audio_format_t& GetOutputFormat() const = 0;

	virtual void SetInputFormat(const audio_format_t& input_fromat) = 0;
	virtual void SetOutputFormat(const audio_format_t& output_fromat) = 0;

};

} // audio

} // media

} // core


#endif // AUDIO_FORMATTER_H
