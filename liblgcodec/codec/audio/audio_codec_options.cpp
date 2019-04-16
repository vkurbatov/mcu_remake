#include "audio_codec_options.h"

namespace largo
{

namespace codec
{

namespace audio
{

const option_key_t AudioCodecOptions::audio_codec_option_num_channels = "codec.audio.num_channels";
const option_key_t AudioCodecOptions::audio_codec_option_format = "codec.audio.format";

AudioCodecOptions::AudioCodecOptions(const audio_codec_options_t& audio_codec_options) :
	CodecOptions(audio_codec_options.codec_options)
{
	SetOption(audio_codec_option_num_channels, &audio_codec_options.num_channels, sizeof(audio_codec_options.num_channels));
	SetOption(audio_codec_option_format, &audio_codec_options.format, sizeof(audio_codec_options.format));
}

} // audio

} // codec

} // largo
