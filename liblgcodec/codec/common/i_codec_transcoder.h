#ifndef I_CODEC_TRANSCODER_H
#define I_CODEC_TRANSCODER_H

#include <cstdint>

namespace largo
{

namespace codec
{

enum class transcoder_direction_t
{
	unknown_transcoder = -1,
	encoder_direction,
	decoder_direction
};

class ICodecTranscoder
{
public:
	virtual ~ICodecTranscoder() {}

	virtual std::int32_t operator()(const void* input_data
							   , std::size_t input_size
							   , void* output_data
							   , std::size_t output_size) = 0;

	virtual transcoder_direction_t GetTranscodeDirection() const = 0;

	// virtual void Reset() = 0;
};

}

}

#endif // I_CODEC_TRANSCODER_H
