#include "audio_packetizer.h"
#include <cstring>

namespace largo
{

namespace codec
{

namespace audio
{

AudioPacketizer::AudioPacketizer(bool is_packetizer)
	: m_is_packetizer(is_packetizer)
{

}

std::size_t AudioPacketizer::Push(const void* data, std::size_t size)
{
	std::size_t result = 0;
	std::size_t process_size = 0;

	do
	{
		size -= process_size;
		data = static_cast<const std::uint8_t*>(data) + process_size;

		process_size = internal_push(data, size);

		result += process_size;
	}
	while(process_size > 0 && process_size < size);

	return result;
}

std::size_t AudioPacketizer::Pop(void* data, std::size_t size)
{
	return internal_pop(data, size);
}

void AudioPacketizer::Reset()
{
	while (!m_packet_queue.empty())
	{
		m_packet_queue.pop();
	}
}

std::size_t AudioPacketizer::Count() const
{
	return m_packet_queue.size();
}

packetizer_direction_t AudioPacketizer::GetDirection() const
{
	return m_is_packetizer ? packetizer_direction_t::packetizer : packetizer_direction_t::depacketizer;
}

std::size_t AudioPacketizer::internal_push(const void* data, std::size_t size)
{
	std::size_t result = 0;

	if (data != nullptr && size > 0)
	{
		auto packet_size = internal_get_packet_size(data, size);

		if (packet_size > 0)
		{
			packet_t packet(packet_size);

			result = internal_packetize(data, size, packet.data(), packet_size);

			if (result > 0)
			{
				m_packet_queue.emplace(std::move(packet));
			}
		}
	}

	return result;
}

std::size_t AudioPacketizer::internal_pop(void* data, std::size_t size)
{
	std::size_t result = 0;

	if (!m_packet_queue.empty())
	{
		auto& packet = m_packet_queue.front();
		result =  packet.size();

		if (data != nullptr)
		{
			result = std::min(result, size);

			if (result > 0)
			{
				std::memcpy(data, packet.data(), result);
			}

			m_packet_queue.pop();
		}
	}

	return result;
}


} // audio

} // codec

} // largo
