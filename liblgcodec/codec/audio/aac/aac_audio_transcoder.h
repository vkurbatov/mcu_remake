#ifndef AAC_AUDIO_TRANSCODER_H
#define AAC_AUDIO_TRANSCODER_H

#include "codec/audio/libav/libav_audio_transcoder.h"
#include "codec/audio/aac/aac_types.h"

namespace largo
{

namespace codec
{

namespace audio
{

class AacAudioTranscoder : public LibavAudioTranscoder
{
public:
	AacAudioTranscoder(bool is_encoder, const aac_profile_id_t& aac_profile, std::uint32_t sample_rate);

};

} // audio

} // codec

} // largo

#endif // ACC_AUDIO_TRANSCODER_H
