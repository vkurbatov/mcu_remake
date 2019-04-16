#ifndef AUDIO_CODEC_OPTIONS_H
#define AUDIO_CODEC_OPTIONS_H

#include "codec/common/codec_options.h"

namespace largo
{

namespace codec
{

namespace audio
{

#ifndef sample_format_t

enum class sample_fromat_t
{
	unknown = -1,
	pcm8,
	pcm16,
	pcm32,
	float32,
	float64
};

#endif

struct audio_codec_options_t
{
	codec_options_t	codec_options;
	std::uint32_t num_channels;
	sample_fromat_t format;
};

class AudioCodecOptions : public CodecOptions
{
public:
	static const option_key_t audio_codec_option_num_channels;
	static const option_key_t audio_codec_option_format;
public:
	AudioCodecOptions(const audio_codec_options_t& audio_codec_options);
	virtual ~AudioCodecOptions() override {}
};

} // audio

} // codec

} // largo


#endif // AUDIO_CODEC_OPTIONS_H
