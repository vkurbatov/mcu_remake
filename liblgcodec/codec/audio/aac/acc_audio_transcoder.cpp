#include "acc_audio_transcoder.h"

namespace largo
{

namespace codec
{

namespace audio
{

namespace aac_utils
{

} // aac_utils

AccAudioTranscoder::AccAudioTranscoder(const aac_profile_id_t& aac_profile, bool is_encoder, std::uint32_t sample_rate)
	: LibavAudioTranscoder(audio_codec_id_t::audio_codec_aac_ld, is_encoder, GetOptions())
{

}

} // audio

} // codec

} // largo
