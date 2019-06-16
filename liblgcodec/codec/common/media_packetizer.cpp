#include "media_packetizer.h"
#include <cstring>

namespace largo
{

namespace codec
{

namespace audio
{

MediaPacketizer::MediaPacketizer(bool is_packetizer)
	: m_is_packetizer(is_packetizer)
{

}

std::size_t MediaPacketizer::Push(const void* data, std::size_t size)
{
	return internal_push(data, size);
}

std::size_t MediaPacketizer::Pop(void* data, std::size_t size)
{
	return internal_pop(data, size);
}

void MediaPacketizer::Reset()
{
	internal_reset();
}

std::size_t MediaPacketizer::Count() const
{
	return internal_count();
}

packetizer_direction_t MediaPacketizer::GetDirection() const
{
	return m_is_packetizer ? packetizer_direction_t::packetizer : packetizer_direction_t::depacketizer;
}

} // audio

} // codec

} // largo
