#ifndef AU_PACKET_H
#define AU_PACKET_H

#include "codec/audio/aac/au_types.h"

#include <queue>
#include <vector>

namespace largo
{

namespace codec
{

namespace audio
{

class AuPacketizer
{

	using packet_t = std::vector<std::uint8_t>;
	using queue_t = std::queue<packet_t>;

	au_header_config_t	m_au_header_config;
	queue_t				m_packet_queue;

public:
	AuPacketizer(const au_header_config_t& au_header_config);

	std::size_t PushFrame(const void* frame, std::size_t size);
	std::size_t PopFrame(void* frame = nullptr, std::size_t size = 0);
	bool DropFrame();

	std::size_t PushPacket(void* packet, std::size_t size);
	std::size_t PopPacket(void* packet, std::size_t size);

	std::size_t Count() const;
	void SetConfig(const au_header_config_t& au_header_config);
	const au_header_config_t& GetConfig() const;
	std::size_t Clear();

private:

	std::size_t get_need_packet_size(std::size_t available_size) const;

};

} // audio

} // codec

} // largo

#endif // AU_PACKET_H
