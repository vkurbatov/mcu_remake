#ifndef ACC_AUDIO_TRANSCODER_H
#define ACC_AUDIO_TRANSCODER_H

#include "codec/audio/libav/libav_audio_transcoder.h"

namespace largo
{

namespace codec
{

namespace audio
{

enum class aac_profile_id_t
{
	aac_profile_ld,
	aac_profile_eld
};

class AccAudioTranscoder : public LibavAudioTranscoder
{
public:
	static AudioCodecOptions CreateAacOptions(const aac_profile_id_t& aac_profile, std::uint32_t sample_rate);
	AccAudioTranscoder(bool is_encoder, const aac_profile_id_t& aac_profile, std::uint32_t sample_rate);
};

} // audio

} // codec

} // largo

#endif // ACC_AUDIO_TRANSCODER_H
