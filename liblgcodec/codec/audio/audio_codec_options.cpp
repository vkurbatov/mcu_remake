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

#define GET_AUDIO_OPTION(param) GET_OPTION(audio_codec_option_, options, audio_codec_options, param)
#define SET_AUDIO_OPTION(param) SET_OPTION(audio_codec_option_, options, audio_codec_options, param)


bool AudioCodecOptions::GetAudioCodecOptions(const IOptions& options, audio_codec_options_t& audio_codec_options)
{
	return
	GET_AUDIO_OPTION(sample_rate) &
	GET_AUDIO_OPTION(bit_rate) &
	GET_AUDIO_OPTION(format) &
	GET_AUDIO_OPTION(num_channels);
}

void AudioCodecOptions::SetAudioCodecOptions(IOptions& options, const audio_codec_options_t& audio_codec_options)
{
	SET_AUDIO_OPTION(sample_rate);
	SET_AUDIO_OPTION(bit_rate);
	SET_AUDIO_OPTION(format);
	SET_AUDIO_OPTION(num_channels);
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
	return OptionsReader::GetOption<std::uint32_t>(*this, audio_codec_option_sample_rate, default_sample_rate);
}

void AudioCodecOptions::SetSampleRate(uint32_t sample_rate)
{
	OptionsWriter::SetOption<std::uint32_t>(*this, audio_codec_option_sample_rate, sample_rate);
}

uint32_t AudioCodecOptions::GetBitRate(uint32_t default_bit_rate) const
{
	return OptionsReader::GetOption<std::uint32_t>(*this, audio_codec_option_bit_rate, default_bit_rate);
}

void AudioCodecOptions::SetBitRate(uint32_t bit_rate)
{
	OptionsWriter::SetOption<std::uint32_t>(*this, audio_codec_option_bit_rate, bit_rate);
}

uint32_t AudioCodecOptions::GetChannels(uint32_t default_channels) const
{
	return OptionsReader::GetOption<std::uint32_t>(*this, audio_codec_option_num_channels, default_channels);
}

void AudioCodecOptions::SetChannels(uint32_t channels)
{
	OptionsWriter::SetOption<std::uint32_t>(*this, audio_codec_option_num_channels, channels);
}

sample_format_t AudioCodecOptions::GetSampleFormat(sample_format_t default_format) const
{
	return OptionsReader::GetOption<sample_format_t>(*this, audio_codec_option_format, default_format);
}

void AudioCodecOptions::SetSampleFormat(sample_format_t sample_format)
{
	OptionsWriter::SetOption<sample_format_t>(*this, audio_codec_option_format, sample_format);
}

} // audio

} // codec

} // largo
