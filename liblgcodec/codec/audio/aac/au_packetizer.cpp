#include "codec/audio/aac/au_packetizer.h"
#include "common/bit_stream.h"
#include "common/bit_stream.tpp"

#include <cstring>

namespace largo
{

namespace codec
{

namespace audio
{

namespace au_packetizer_utils
{

std::size_t get_au_header_size(const au_header_rules_t& au_rules, bool is_delta = false)
{
	return au_rules.size_length
			+ (is_delta	? au_rules.index_delta_length : au_rules.index_length)
			+ (au_rules.cts_delta_length != 0 ? au_rules.cts_delta_length + 1 : 0)
			+ (au_rules.dts_delta_length != 0 ? au_rules.dts_delta_length + 1 : 0)
			+ au_rules.rap_length
			+ au_rules.stream_state_length;
}

bool fetch_au_header(BitStreamReader& bit_stream, const au_header_rules_t& au_rules, au_header_t& au_header, bool is_delta = false)
{
	std::memset(&au_header, 0, sizeof(au_header_t));

	bit_stream.Read(&au_header.size, au_rules.size_length);
	bit_stream.Read(&au_header.index, is_delta ? au_rules.index_delta_length : au_rules.index_length);

	if (au_rules.cts_delta_length > 0)
	{
		bit_stream.Read(&au_header.cts_flag, 1);
		bit_stream.Read(&au_header.cts_delta, au_rules.cts_delta_length);
	}

	if (au_rules.dts_delta_length > 0)
	{
		bit_stream.Read(&au_header.dts_flag, 1);
		bit_stream.Read(&au_header.dts_delta, au_rules.dts_delta_length);
	}

	bit_stream.Read(&au_header.rap_flag, au_rules.rap_length);
	bit_stream.Read(&au_header.stream_state, au_rules.stream_state_length);

	return au_header.is_valid();
}

bool put_au_header(BitStreamWriter& bit_stream, const au_header_rules_t& au_rules, const au_header_t& au_header, bool is_delta = false)
{

	bit_stream.Write(&au_header.size, au_rules.size_length);
	bit_stream.Write(&au_header.index, is_delta ? au_rules.index_delta_length : au_rules.index_length);

	if (au_rules.cts_delta_length > 0)
	{
		bit_stream.Write(&au_header.cts_flag, 1);
		bit_stream.Write(&au_header.cts_delta, au_rules.cts_delta_length);
	}

	if (au_rules.dts_delta_length > 0)
	{
		bit_stream.Write(&au_header.dts_flag, 1);
		bit_stream.Write(&au_header.dts_delta, au_rules.dts_delta_length);
	}

	bit_stream.Write(&au_header.rap_flag, au_rules.rap_length);
	bit_stream.Write(&au_header.stream_state, au_rules.stream_state_length);

	return au_header.is_valid();
}

std::size_t au_depacketize(const au_header_rules_t& au_rules, const void* packet, std::size_t size, AuPacketizer::queue_t& frame_queue)
{
	std::size_t result = 0;

	std::size_t bit_size = size * 8;

	if (bit_size >= au_header_section_size_length)
	{
		BitStreamReader bit_reader(packet);

		std::size_t au_header_section_size = bit_reader.Read<std::size_t>(au_header_section_size_length);

		if (au_header_section_size > 0)
		{
			if ((au_header_section_size + au_header_section_size_length) <= bit_size)
			{

				const std::size_t data_section_pos = (au_header_section_size_length + au_header_section_size + 7) / 8;

				auto au_data_ptr = static_cast<const std::uint8_t*>(packet) + (data_section_pos);
				auto au_data_size = size - data_section_pos;

				bool is_complete = true;

				do
				{
					au_header_t au_header = {};
					is_complete = true;

					if (fetch_au_header(bit_reader, au_rules, au_header, result > 0))
					{
						if (au_header.size <= au_data_size)
						{
							frame_queue.emplace(au_data_ptr, au_data_ptr + au_header.size);

							au_data_ptr += au_header.size;
							au_data_size -= au_header.size;

							is_complete = au_data_size == 0;

							result++;

						}
					}
				}
				while(is_complete == false);
			}
		}
	}

	return result;
}

std::size_t au_packetizer(const au_header_rules_t& au_rules, void* packet, std::size_t size, AuPacketizer::queue_t& frame_queue)
{
	std::size_t result = 0;
	std::size_t bit_size = size * 8;

	if (bit_size > au_header_section_size_length && !frame_queue.empty())
	{
		std::uint16_t frame_number = 0;
		std::size_t au_data_size = 0;
		std::size_t au_header_section_size = 0;
		AuPacketizer::queue_t au_data_queue;

		BitStreamWriter bit_writer(packet);
		bit_writer.Reset(au_header_section_size_length);

		bool is_complete;

		do
		{
			is_complete = true;
		}
		while(is_complete == false);
	}

	return result;
}

}

//---------------------------------------------------

AuPacketizer::AuPacketizer(const au_header_rules_t &au_header_config)
 : m_au_header_rules(au_header_config)
{

}

std::size_t AuPacketizer::PushFrame(const void *frame, std::size_t size)
{

	std::size_t result = 0;

	if (frame != nullptr && size > 0)
	{
		m_frame_queue.emplace(static_cast<const std::uint8_t*>(frame)
						   , static_cast<const std::uint8_t*>(frame) + size);
		result = size;
	}
}

std::size_t AuPacketizer::PopFrame(void *frame, std::size_t size)
{
	std::size_t result = 0;

	if (!m_frame_queue.empty())
	{
		const auto& packet = m_frame_queue.front();
		result = packet.size();

		if (frame != nullptr)
		{
			result = std::min(result, size);
			std::memcpy(frame, packet.data(), result);
			m_frame_queue.pop();
		}
	}

	return result;
}

bool AuPacketizer::DropFrame()
{
	bool result = !m_frame_queue.empty();

	if (result == true)
	{
		m_frame_queue.pop();
	}

	return result;
}

std::size_t AuPacketizer::PushPacket(void *packet, std::size_t size)
{
	return au_packetizer_utils::au_depacketize(m_au_header_rules, packet, size, m_frame_queue);
}

std::size_t AuPacketizer::PopPacket(void *packet, std::size_t size)
{

}

std::size_t AuPacketizer::Count() const
{
	return m_frame_queue.size();
}

void AuPacketizer::SetRules(const au_header_rules_t& au_header_config)
{
	m_au_header_rules = au_header_config;
}

const au_header_rules_t& AuPacketizer::GetRules() const
{
	return m_au_header_rules;
}

std::size_t AuPacketizer::Clear()
{
	auto result = Count();

	while (DropFrame());

	return result;
}


std::size_t AuPacketizer::get_need_packet_size(std::size_t available_size) const
{
	std::size_t result = 0;



	return result;
}

} // audio

} // codec

} // largo
