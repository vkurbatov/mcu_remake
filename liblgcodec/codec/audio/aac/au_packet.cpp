#include "au_packet.h"

#include <cstring>

namespace largo
{

namespace codec
{

namespace audio
{


//---------------------------------------------------

AuPacketizer::AuPacketizer(const au_header_config_t &au_header_config)
 : m_au_header_config(au_header_config)
{

}

std::size_t AuPacketizer::PushFrame(const void *frame, std::size_t size)
{

	std::size_t result = 0;

	if (frame != nullptr && size > 0)
	{
		m_packet_queue.emplace(static_cast<const std::uint8_t*>(frame)
						   , static_cast<const std::uint8_t*>(frame) + size);
		result = size;
	}
}

std::size_t AuPacketizer::PopFrame(void *frame, std::size_t size)
{
	std::size_t result = 0;

	if (!m_packet_queue.empty())
	{
		const auto& packet = m_packet_queue.front();
		result = packet.size();

		if (frame != nullptr)
		{
			result = std::min(result, size);
			std::memcpy(frame, packet.data(), result);
			m_packet_queue.pop();
		}
	}

	return result;
}

bool AuPacketizer::DropFrame()
{
	bool result = !m_packet_queue.empty();

	if (result == true)
	{
		m_packet_queue.pop();
	}

	return result;
}

std::size_t AuPacketizer::PushPacket(void *packet, std::size_t size)
{

}

std::size_t AuPacketizer::PopPacket(void *packet, std::size_t size)
{

}

std::size_t AuPacketizer::Count() const
{
	return m_packet_queue.size();
}

void AuPacketizer::SetConfig(const au_header_config_t& au_header_config)
{
	m_au_header_config = au_header_config;
}

const au_header_config_t& AuPacketizer::GetConfig() const
{
	return m_au_header_config;
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
