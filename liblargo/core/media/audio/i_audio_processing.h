#ifndef I_AUDIO_PROCESSING_H
#define I_AUDIO_PROCESSING_H

#include "core/media/audio/audio_format.h"

namespace core
{

namespace media
{

namespace audio
{

class IAudioProcessing
{
public:
	virtual ~IAudioProcessing() {}

	virtual std::size_t AudioProcessingRead(const audio_format_t& audio_format, const void* input_data, std::size_t input_size, void* output_data, std::size_t output_size) = 0;
	virtual std::size_t AudioProcessingWrite(const audio_format_t& audio_format, const void* input_data, std::size_t input_size, void* output_data, std::size_t output_size) = 0;

	virtual bool CanReadProcessing() const = 0;
	virtual bool CanWriteProcessing() const = 0;

	virtual void Reset() = 0;
};

} // audio

} // media

} // core
#endif // I_AUDIO_PROCESSING_H
