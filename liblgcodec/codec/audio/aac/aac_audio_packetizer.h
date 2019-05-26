#ifndef AAC_AUDIO_PACKETIZER_H
#define AAC_AUDIO_PACKETIZER_H

#include "codec/audio/audio_packetizer.h"
#include "codec/audio/aac/aac_types.h"

namespace largo
{

namespace codec
{

namespace audio
{

class AacAudioPacketizer : public AudioPacketizer
{

public:
	AacAudioPacketizer(bool is_packetizer);
	virtual ~AacAudioPacketizer() {}

	// AudioPacketizer interface
protected:
	std::size_t internal_get_packet_size(const void *data, std::size_t size) const override;
	std::size_t internal_packetize(const void *input_data, std::size_t input_size, void *output_data, std::size_t output_size) override;

};

} // audio

} // codec

} // largo

#endif // AAC_AUDIO_PACKETIZER_H
