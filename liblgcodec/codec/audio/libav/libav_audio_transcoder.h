#ifndef LIBAVAUDIOTRANSCODER_H
#define LIBAVAUDIOTRANSCODER_H

#include "codec/audio/audio_codec.h"
#include "codec/audio/libav/libav_wrapper.h"

namespace largo
{

namespace codec
{

namespace audio
{

class LibavAudioTranscoder : public AudioCodec
{
	audio_codec_id_t				m_codec_id;
	libav_wrapper_ptr				m_av_codec;

public:
	static const option_key_t libav_audio_codec_option_frame_size;
	static const option_key_t libav_audio_codec_option_profile;

public:
	LibavAudioTranscoder(audio_codec_id_t codec_id, bool is_encoder);
	virtual ~LibavAudioTranscoder();

	// AudioCodec interface
protected:
	bool internal_open() override;
	bool internal_close() override;
	bool internal_reconfigure(AudioCodecOptions& audio_codec_options) override;
	std::int32_t internal_transcode(const void* input_data, std::size_t input_size, void* output_data, std::size_t output_size) override;

	// IAudioCodec interface
public:
	audio_codec_id_t GetCodecId() const override;

};

} // audio

} // codec

} // largo

#endif // LIBAVAUDIOTRANSCODER_H
