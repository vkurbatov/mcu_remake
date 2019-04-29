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
	AacAudioCodecOptions(const aac_profile_id_t& aac_profile_id, std::uint32_t sample_rate = default_sample_rate);

	aac_profile_id_t GetAacProfileId(const aac_profile_id_t& default_aac_profile_id = aac_profile_id_t::aac_profile_ld) const;
	void SetAacProfileId(const aac_profile_id_t& aac_profile_id);
};

} // audio

} // codec

} // largo

#endif // AAC_AUDIO_CODEC_OPTIONS_H
