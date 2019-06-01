#include "au_packet.h"

#include <cstring>

namespace largo
{

namespace codec
{

namespace audio
{

AuPacketizer::AuPacketizer(const au_header_config_t &au_header_config)
 : m_au_header_config(au_header_config)
{

}

std::size_t AuPacketizer::PushData(const void *data, std::size_t size)
{

	std::size_t result = 0;

	if (data != nullptr && size > 0)
	{
		m_packet_queue.emplace(static_cast<const std::uint8_t*>(data)
						   , static_cast<const std::uint8_t*>(data) + size);
		result = size;
	}
}

std::size_t AuPacketizer::PopData(void *data, std::size_t size)
{
	std::size_t result = 0;

	if (!m_packet_queue.empty())
	{
		const auto& packet = m_packet_queue.front();
		result = packet.size();

		if (data != nullptr)
		{
			result = std::min(result, size);
			std::memcpy(data, packet.data(), result);
			m_packet_queue.pop();
		}
	}

	return result;
}

bool AuPacketizer::DropData()
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

	while (DropData());

	return result;
}

std::size_t AuPacketizer::get_need_packet_size(std::size_t available_size) const
{

}


} // audio

} // codec

} // largo
