#include "aac_audio_packetizer.h"
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

const aac_packetize_options_t &AacAudioPacketizer::GetPacketizeOptions() const
{
	return m_aac_packetize_options;
}

std::size_t AacAudioPacketizer::internal_get_packet_size(const void *data, std::size_t size) const
{
	bool is_packetize = GetDirection() == packetizer_direction_t::packetizer;

}

std::size_t AacAudioPacketizer::internal_packetize(const void *input_data, std::size_t input_size, void *output_data, std::size_t output_size)
{
	bool is_packetize = GetDirection() == packetizer_direction_t::packetizer;
}

} // audio

} // codec

} // largo
