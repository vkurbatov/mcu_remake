#include "aac_audio_transcoder.h"
#include "aac_audio_codec_options.h"

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


} // aac_utils

AacAudioTranscoder::AacAudioTranscoder(bool is_encoder, const aac_profile_id_t& aac_profile, std::uint32_t sample_rate)
	: LibavAudioTranscoder(audio_codec_id_t::audio_codec_aac, is_encoder, AacAudioCodecOptions(aac_profile, sample_rate))
{

}


} // audio

} // codec

} // largo
