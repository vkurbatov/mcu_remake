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

const option_key_t AacAudioCodecOptions::audio_codec_aac_option_header_size_length = "codec.audio.aac.header.size_lenght";
const option_key_t AacAudioCodecOptions::audio_codec_aac_option_header_index_length = "codec.audio.aac.header.index_delta_lenght";
const option_key_t AacAudioCodecOptions::audio_codec_aac_option_header_index_delta_length = "codec.audio.aac.header.index_delta_lenght";
const option_key_t AacAudioCodecOptions::audio_codec_aac_option_header_cts_delta_length = "codec.audio.aac.header.cts_delay_lenght";
const option_key_t AacAudioCodecOptions::audio_codec_aac_option_header_dts_delta_length = "codec.audio.aac.header.dts_delay_lenght";
const option_key_t AacAudioCodecOptions::audio_codec_aac_option_header_rap_length = "codec.audio.aac.header.rap_lenght";
const option_key_t AacAudioCodecOptions::audio_codec_aac_option_header_stream_state_length = "codec.audio.aac.header.stream_state_lenght";

#define GET_AAC_RULES_OPTION(param) GET_OPTION(audio_codec_aac_option_header_, options, aac_header_rules, param)
#define SET_AAC_RULES_OPTION(param) SET_OPTION(audio_codec_aac_option_header_, options, aac_header_rules, param)

bool AacAudioCodecOptions::GetAacHeaderRules(const IOptions &options, aac_header_rules_t& aac_header_rules)
{
	return
	GET_AAC_RULES_OPTION(size_length) &
	GET_AAC_RULES_OPTION(index_length) &
	GET_AAC_RULES_OPTION(index_delta_length) &
	GET_AAC_RULES_OPTION(cts_delta_length) &
	GET_AAC_RULES_OPTION(dts_delta_length) &
	GET_AAC_RULES_OPTION(rap_length) &
	GET_AAC_RULES_OPTION(stream_state_length);
}

void AacAudioCodecOptions::SetAacHeaderRules(IOptions &options, const aac_header_rules_t &aac_header_rules)
{
	SET_AAC_RULES_OPTION(size_length);
	SET_AAC_RULES_OPTION(index_length);
	SET_AAC_RULES_OPTION(index_delta_length);
	SET_AAC_RULES_OPTION(cts_delta_length);
	SET_AAC_RULES_OPTION(dts_delta_length);
	SET_AAC_RULES_OPTION(rap_length);
	SET_AAC_RULES_OPTION(stream_state_length);
}

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

aac_header_rules_t AacAudioCodecOptions::GetAacHeaderRules() const
{

}

void AacAudioCodecOptions::SetAacHeaderRules(const aac_header_rules_t &aac_header_rules_t) const
{

}

} // audio

} // codec

} // largo

