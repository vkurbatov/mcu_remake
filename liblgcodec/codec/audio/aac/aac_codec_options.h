#ifndef AAC_CODEC_OPTIONS_H
#define AAC_CODEC_OPTIONS_H

#include "codec/audio/audio_codec_options.h"

namespace largo
{

namespace codec
{

namespace audio
{

namespace aac
{

struct aac_audio_codec_options_t
{
	audio_codec_options_t audio_codec_options;
	std::uint64_t profile;
};

class AacCodecOptions: public AudioCodecOptions
{
public:
	static const option_key_t aac_audio_codec_option_profile;
public:
	AacCodecOptions(const aac_audio_codec_options_t& aac_audio_codec_options);
	virtual ~AacCodecOptions() override {}
};

} // aac

} // audio

} // codec

} // largo

#endif // AAC_CODEC_OPTIONS_H
