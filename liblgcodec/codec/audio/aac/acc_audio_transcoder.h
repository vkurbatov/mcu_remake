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
	AccAudioTranscoder(const aac_profile_id_t& aac_profile, bool is_encoder, std::uint32_t sample_rate);
};

} // audio

} // codec

} // largo

#endif // ACC_AUDIO_TRANSCODER_H
