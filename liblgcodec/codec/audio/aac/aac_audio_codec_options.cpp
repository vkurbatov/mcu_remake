#include "aac_audio_codec_options.h"

extern "C"
{
#include <libavcodec/avcodec.h>
}

#include "codec/audio/libav/libav_audio_transcoder.h"

namespace largo
{

namespace codec
{

namespace audio
{


AacAudioCodecOptions::AacAudioCodecOptions(const aac_profile_id_t& aac_profile_id, std::uint32_t sample_rate)
	: AudioCodecOptions(sample_rate, 0, 1, sample_format_t::float_32)
{
	SetAacProfileId(aac_profile_id);
}

aac_profile_id_t AacAudioCodecOptions::GetAacProfileId(const aac_profile_id_t& default_aac_profile_id) const
{
	aac_profile_id_t result = aac_profile_id_t::aac_profile_unknown;

	std::int32_t av_profile = FF_PROFILE_UNKNOWN;

	GetOption(LibavAudioTranscoder::libav_audio_codec_option_profile, &av_profile);

	switch(av_profile)
	{
		case FF_PROFILE_AAC_LD:
			result = aac_profile_id_t::aac_profile_ld;
		break;

		case FF_PROFILE_AAC_ELD:
			result = aac_profile_id_t::aac_profile_eld;
		break;
	}

	return result;
}

void AacAudioCodecOptions::SetAacProfileId(const aac_profile_id_t& aac_profile_id)
{
	std::int32_t av_profile = FF_PROFILE_UNKNOWN;

	switch(aac_profile_id)
	{
		case aac_profile_id_t::aac_profile_ld:
			av_profile = FF_PROFILE_AAC_LD;
		break;
		case aac_profile_id_t::aac_profile_eld:
			av_profile = FF_PROFILE_AAC_ELD;
		break;
	}

	SetOption(LibavAudioTranscoder::libav_audio_codec_option_profile, &av_profile, sizeof(av_profile));
}

} // audio

} // codec

} // largo

