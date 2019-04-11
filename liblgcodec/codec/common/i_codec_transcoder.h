#ifndef I_CODEC_TRANSCODER_H
#define I_CODEC_TRANSCODER_H

#include <cstdint>

namespace largo
{

namespace codec
{

class ICodecTranscoder
{
public:
	virtual ~ICodecTranscoder() {}

	virtual operator()(const void* input_data
							   , std::size_t input_size
							   , void* output_data
							   , std::size_t output_size) = 0;
};

}

}

#endif // I_CODEC_TRANSCODER_H
