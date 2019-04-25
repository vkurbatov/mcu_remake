#ifndef AUDIO_CODEC_OPTIONS_H
#define AUDIO_CODEC_OPTIONS_H

#include "common/options.h"
#include "codec/audio/audio_codec_types.h"

namespace largo
{

namespace codec
{

namespace audio
{

const std::uint32_t default_sample_rate = 8000;
const std::uint32_t default_bit_rate = 0;
const std::uint32_t default_num_channels = 1;

struct audio_codec_options_t
{
	std::uint32_t sample_rate;
	std::uint32_t bit_rate;
	std::uint32_t num_channels;
	sample_format_t format;
};

const audio_codec_options_t default_audio_codec_options =
	{ default_sample_rate, default_bit_rate, default_num_channels, default_sample_format };

class AudioCodecOptions : virtual public Options
{
public:
	static const option_key_t audio_codec_option_sample_rate;
	static const option_key_t audio_codec_option_bit_rate;
	static const option_key_t audio_codec_option_num_channels;
	static const option_key_t audio_codec_option_format;

	static bool GetAudioCodecOptions(const IOptions& options, audio_codec_options_t& audio_codec_options);
	static void SetAudioCodecOptions(IOptions& options, const audio_codec_options_t& audio_codec_options);

public:
	AudioCodecOptions(const audio_codec_options_t& audio_codec_options = default_audio_codec_options);
	virtual ~AudioCodecOptions() override {}

	bool GetAudioCodecOptions(audio_codec_options_t& audio_codec_options) const;
	void SetAudioCodecOptions(const audio_codec_options_t& audio_codec_options);
};

} // audio

} // codec

} // largo


#endif // AUDIO_CODEC_OPTIONS_H
