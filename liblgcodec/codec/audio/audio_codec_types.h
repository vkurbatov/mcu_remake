#ifndef AUDIO_CODEC_TYPES_H
#define AUDIO_CODEC_TYPES_H

namespace largo
{

namespace codec
{

namespace audio
{

enum class sample_format_t
{
	unknown,
	pcm_8,
	pcm_16,
	pcm_32,
	float_32,
	float_64
};

const sample_format_t default_sample_format = sample_format_t::pcm_16;

enum class audio_codec_id_t
{
	audio_codec_unknown,
	audio_codec_g723_1,
	audio_codec_aac
};


} // audio

} // codec

} // largo

#endif // AUDIO_CODEC_TYPES_H
