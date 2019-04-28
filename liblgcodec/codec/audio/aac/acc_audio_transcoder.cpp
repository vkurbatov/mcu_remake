#include "acc_audio_transcoder.h"
#include "common/options_helper.h"

extern "C"
{
#include <libavcodec/avcodec.h>
}

namespace largo
{

namespace codec
{

namespace audio
{

namespace aac_utils
{

std::int32_t aac_profile_to_av_profile(const aac_profile_id_t &aac_profile)
{
	std::int32_t result = FF_PROFILE_UNKNOWN;

	switch(aac_profile)
	{
		case aac_profile_id_t::aac_profile_ld:
			result = FF_PROFILE_AAC_LD;
			break;
		case aac_profile_id_t::aac_profile_eld:
			result = FF_PROFILE_AAC_ELD;
		break;
	}

	return result;
}

} // aac_utils

AudioCodecOptions AccAudioTranscoder::CreateAacOptions(const aac_profile_id_t &aac_profile, uint32_t sample_rate)
{
	audio_codec_options_t audio_options = default_audio_codec_options;

	audio_options.sample_rate = sample_rate;
	audio_options.bit_rate = 48000;
	audio_options.format = sample_format_t::float_32;
	audio_options.num_channels = 1;

	AudioCodecOptions audio_codec_options(audio_options);

	auto av_profile = aac_utils::aac_profile_to_av_profile(aac_profile);

	audio_codec_options.SetOption(libav_audio_codec_option_profile, &av_profile, sizeof(av_profile));

	return std::move(audio_codec_options);
}

AccAudioTranscoder::AccAudioTranscoder(bool is_encoder, const aac_profile_id_t& aac_profile, std::uint32_t sample_rate)
	: LibavAudioTranscoder(audio_codec_id_t::audio_codec_aac, is_encoder, CreateAacOptions(aac_profile, sample_rate))
{

}

} // audio

} // codec

} // largo
