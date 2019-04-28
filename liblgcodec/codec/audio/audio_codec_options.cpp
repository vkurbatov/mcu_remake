#include "audio_codec_options.h"
#include "common/options_helper.h"

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

AudioCodecOptions::AudioCodecOptions(uint32_t sample_rate, uint32_t bit_rate, uint32_t channels, sample_format_t sample_format)
{
	SetAudioCodecOptions( { sample_rate, bit_rate, channels, sample_format });
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

uint32_t AudioCodecOptions::GetSampleRate(uint32_t default_sample_rate) const
{
	return OptionsHelper::GetOption<std::uint32_t>(*this, audio_codec_option_sample_rate, default_sample_rate);
}

void AudioCodecOptions::SetSampleRate(uint32_t sample_rate)
{
	OptionsHelper::SetOption<std::uint32_t>(*this, audio_codec_option_sample_rate, sample_rate);
}

uint32_t AudioCodecOptions::GetBitRate(uint32_t default_bit_rate) const
{
	return OptionsHelper::GetOption<std::uint32_t>(*this, audio_codec_option_bit_rate, default_bit_rate);
}

void AudioCodecOptions::SetBitRate(uint32_t bit_rate)
{
	OptionsHelper::SetOption<std::uint32_t>(*this, audio_codec_option_bit_rate, bit_rate);
}

uint32_t AudioCodecOptions::GetChannels(uint32_t default_channels) const
{
	return OptionsHelper::GetOption<std::uint32_t>(*this, audio_codec_option_num_channels, default_channels);
}

void AudioCodecOptions::SetChannels(uint32_t channels)
{
	OptionsHelper::SetOption<std::uint32_t>(*this, audio_codec_option_num_channels, channels);
}

sample_format_t AudioCodecOptions::GetSampleFormat(sample_format_t default_format) const
{
	return OptionsHelper::GetOption<sample_format_t>(*this, audio_codec_option_format, default_format);
}

void AudioCodecOptions::SetSampleFormat(sample_format_t sample_format)
{
	OptionsHelper::SetOption<sample_format_t>(*this, audio_codec_option_format, sample_format);
}

} // audio

} // codec

} // largo
