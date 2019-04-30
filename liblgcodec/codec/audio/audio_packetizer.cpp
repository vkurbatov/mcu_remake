#include "audio_packetizer.h"
#include <cerrno>

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

std::int32_t AudioPacketizer::Push(const void* data, std::size_t size)
{
	std::int32_t result = -EINVAL;

	if (data != nullptr && size > 0)
	{

		auto result = internal_get_packet_size(data, size);

		if (result > 0)
		{
			m_packet_queue.emplace(result);

			auto& packet = m_packet_queue.front();

			result = internal_packetize(data, size, packet.data(), result);
		}
	}

	return result;
}

std::int32_t AudioPacketizer::Pop(void* data, std::size_t size)
{

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


} // audio

} // codec

} // largo
