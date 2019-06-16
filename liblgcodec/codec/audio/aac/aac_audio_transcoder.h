#ifndef AAC_AUDIO_TRANSCODER_H
#define AAC_AUDIO_TRANSCODER_H

#include "codec/audio/libav/libav_audio_transcoder.h"
#include "codec/audio/aac/aac_types.h"
#include "codec/audio/aac/aac_audio_packetizer.h"

namespace largo
{

namespace codec
{

namespace audio
{

class AacAudioTranscoder : public LibavAudioTranscoder
{
	AacAudioPacketizer			m_aac_packetizer;
	std::vector<std::uint8_t>	m_transcode_buffer;

public:
	AacAudioTranscoder(bool is_encoder, const aac_profile_id_t& aac_profile, std::uint32_t sample_rate);

protected:
	virtual bool internal_reconfigure(AudioCodecOptions &audio_codec_options) override;
	virtual std::int32_t internal_transcode(const void *input_data, std::size_t input_size, void *output_data, std::size_t output_size) override;

private:
	void prepare_buffer(std::size_t size);
};

} // audio

} // codec

} // largo

#endif // ACC_AUDIO_TRANSCODER_H
