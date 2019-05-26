#include "aac_audio_packetizer.h"
#include "common/bit_stream.h"

namespace largo
{

namespace codec
{

namespace audio
{

AacAudioPacketizer::AacAudioPacketizer(bool is_packetizer)
	: AudioPacketizer(is_packetizer)
{

}

std::size_t AacAudioPacketizer::internal_get_packet_size(const void *data, std::size_t size) const
{
	std::size_t result = 0;



	return result;
}

std::size_t AacAudioPacketizer::internal_packetize(const void *input_data, std::size_t input_size, void *output_data, std::size_t output_size)
{
	std::size_t result = 0;


	return result;
}

} // audio

} // codec

} // largo
