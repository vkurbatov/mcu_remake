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
	, m_au_packetize_options(default_au_header_options)
	, m_au_index(0)
{

}

const au_header_options_t &AacAudioPacketizer::GetPacketizeOptions() const
{
	return m_au_packetize_options;
}

std::size_t AacAudioPacketizer::internal_get_packet_size(const void *data, std::size_t size) const
{
	std::size_t result = 0;

	switch(GetDirection())
	{
		case packetizer_direction_t::packetizer:
			result = get_au_header_size() + std::min(size, m_max_payload_size);
		break;
		case packetizer_direction_t::depacketizer:

		break;
	}

	return result;
}

std::size_t AacAudioPacketizer::internal_packetize(const void *input_data, std::size_t input_size, void *output_data, std::size_t output_size)
{
	bool is_packetize = GetDirection() == packetizer_direction_t::packetizer;
}

std::size_t AacAudioPacketizer::get_au_header_size(bool first_frame) const
{
	auto au_bit_size = m_au_packetize_options.au_size_length;
	au_bit_size += (first_frame
					? m_au_packetize_options.au_index_length
					: m_au_packetize_options.au_index_delta_length);

	return (au_bit_size + 7) / 8;
}

std::size_t AacAudioPacketizer::get_payload_size(const void* au_header, std::size_t size) const
{
	std::size_t result = 0;

	auto au_header_size = get_au_header_size(is_first_frame());

	if (size > au_header_size)
	{
		BitStreamReader	bit_reader(au_header);

		auto au_size = bit_reader.Read<std::int32_t>(m_au_packetize_options.au_size_length);
		auto au_index = bit_reader.Read<std::int32_t>(is_first_frame() ? m_au_packetize_options.au_index_length : m_au_packetize_options.au_index_delta_length);
	}

	return result;
}

bool AacAudioPacketizer::is_first_frame() const
{
	return m_au_index > 0;
}


} // audio

} // codec

} // largo
