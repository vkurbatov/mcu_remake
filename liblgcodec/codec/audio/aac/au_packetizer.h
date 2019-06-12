#ifndef AU_PACKETIZER_H
#define AU_PACKETIZER_H

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

public:
	using frame_t = std::vector<std::uint8_t>;
	using queue_t = std::queue<frame_t>;

private:
	au_header_rules_t	m_au_header_rules;
	queue_t				m_frame_queue;

public:
	AuPacketizer(const au_header_rules_t& au_header_config);

	std::size_t PushFrame(const void* frame, std::size_t size);
	std::size_t PopFrame(void* frame = nullptr, std::size_t size = 0);
	bool DropFrame();

	std::size_t PushPacket(void* packet, std::size_t size);
	std::size_t PopPacket(void* packet, std::size_t size);

	std::size_t Count() const;
	void SetRules(const au_header_rules_t& au_header_config);
	const au_header_rules_t& GetRules() const;
	std::size_t Clear();

private:

	std::size_t get_need_packet_size(std::size_t available_size) const;

};

} // audio

} // codec

} // largo

#endif // AU_PACKET_H
