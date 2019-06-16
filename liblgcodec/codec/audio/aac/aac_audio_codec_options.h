#ifndef AAC_AUDIO_CODEC_OPTIONS_H
#define AAC_AUDIO_CODEC_OPTIONS_H

#include "codec/audio/audio_codec_options.h"
#include "codec/audio/aac/aac_types.h"

namespace largo
{

namespace codec
{

namespace audio
{

class AacAudioCodecOptions : public AudioCodecOptions
{
public:

	static const option_key_t audio_codec_aac_option_header_size_length;
	static const option_key_t audio_codec_aac_option_header_index_length;
	static const option_key_t audio_codec_aac_option_header_index_delta_length;
	static const option_key_t audio_codec_aac_option_header_cts_delta_length;
	static const option_key_t audio_codec_aac_option_header_dts_delta_length;
	static const option_key_t audio_codec_aac_option_header_rap_length;
	static const option_key_t audio_codec_aac_option_header_stream_state_length;

	static bool GetAacHeaderRules(const IOptions& options, aac_header_rules_t& aac_header_rules);
	static void SetAacHeaderRules(IOptions& options, const aac_header_rules_t& aac_header_rules);

public:
	AacAudioCodecOptions(const aac_profile_id_t& aac_profile_id, std::uint32_t sample_rate = default_sample_rate);

	aac_profile_id_t GetAacProfileId(const aac_profile_id_t& default_aac_profile_id = aac_profile_id_t::aac_profile_ld) const;
	void SetAacProfileId(const aac_profile_id_t& aac_profile_id);

	aac_header_rules_t GetAacHeaderRules() const;
	void SetAacHeaderRules(const aac_header_rules_t& aac_header_rules_t) const;
};

} // audio

} // codec

} // largo

#endif // AAC_AUDIO_CODEC_OPTIONS_H
