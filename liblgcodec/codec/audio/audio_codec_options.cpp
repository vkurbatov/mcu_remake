#include "audio_codec_options.h"

namespace largo
{

namespace codec
{

namespace audio
{

const option_key_t AudioCodecOptions::audio_codec_option_sample_rate = "codec.audio.sample_rate";
const option_key_t AudioCodecOptions::audio_codec_option_bit_rate = "codec.audio.bit_rate";
const option_key_t AudioCodecOptions::audio_codec_option_num_channels = "codec.audio.num_channels";
const option_key_t AudioCodecOptions::audio_codec_option_format = "codec.audio.format";

bool AudioCodecOptions::GetAudioCodecOptions(const IOptions& options, audio_codec_options_t& audio_codec_options)
{

	return options.GetOption(audio_codec_option_sample_rate, &audio_codec_options.sample_rate, sizeof(audio_codec_options.sample_rate)) &
		options.GetOption(audio_codec_option_bit_rate, &audio_codec_options.bit_rate, sizeof(audio_codec_options.bit_rate)) &
		options.GetOption(audio_codec_option_num_channels, &audio_codec_options.format, sizeof(audio_codec_options.format)) &
		options.GetOption(audio_codec_option_format, &audio_codec_options.num_channels, sizeof(audio_codec_options.num_channels));
}

void AudioCodecOptions::SetAudioCodecOptions(IOptions& options, const audio_codec_options_t& audio_codec_options)
{
	options.SetOption(audio_codec_option_sample_rate, &audio_codec_options.sample_rate, sizeof(audio_codec_options.sample_rate));
	options.SetOption(audio_codec_option_bit_rate, &audio_codec_options.bit_rate, sizeof(audio_codec_options.bit_rate));
	options.SetOption(audio_codec_option_num_channels, &audio_codec_options.num_channels, sizeof(audio_codec_options.num_channels));
	options.SetOption(audio_codec_option_format, &audio_codec_options.format, sizeof(audio_codec_options.format));
}

AudioCodecOptions::AudioCodecOptions(const audio_codec_options_t& audio_codec_options)
{
	SetAudioCodecOptions(audio_codec_options);
}

AudioCodecOptions::AudioCodecOptions(const IOptions& options)
{
	audio_codec_options_t audio_codec_options = default_audio_codec_options;

	GetAudioCodecOptions(options, audio_codec_options);

	SetAudioCodecOptions(audio_codec_options);
}

bool AudioCodecOptions::GetAudioCodecOptions(audio_codec_options_t& audio_codec_options) const
{
	return GetAudioCodecOptions(*this, audio_codec_options);
}

void AudioCodecOptions::SetAudioCodecOptions(const audio_codec_options_t& audio_codec_options)
{
	return SetAudioCodecOptions(*this, audio_codec_options);
}

} // audio

} // codec

} // largo
