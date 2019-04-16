#include "aac_codec_options.h"

namespace largo
{

namespace codec
{

namespace audio
{

namespace aac
{

const option_key_t AacCodecOptions::aac_audio_codec_option_profile = "codec.audio.aac.profile";

AacCodecOptions::AacCodecOptions(const aac_audio_codec_options_t& aac_audio_codec_options)
	: AudioCodecOptions(aac_audio_codec_options.audio_codec_options)
{
	SetOption(aac_audio_codec_option_profile, &aac_audio_codec_options.profile, sizeof(aac_audio_codec_options.profile));
}

} // aac

} // audio

} // codec

} // largo
