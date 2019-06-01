#ifndef AU_PACKET_H
#define AU_PACKET_H

#include <cstdint>
#include <queue>
#include <vector>

namespace largo
{

namespace codec
{

namespace audio
{

struct au_header_t
{
	std::uint32_t	size;
	std::uint32_t	index;
	bool			cts_flag;
	std::uint32_t	cts_delta;
	bool			dts_flag;
	std::uint32_t	dts_delta;
	std::uint32_t	rap_flag;
};

struct au_header_config_t
{
	std::uint32_t	au_size_length;
	std::uint32_t	au_index_length;
	std::uint32_t	au_index_delta_length;
	std::uint32_t	au_cts_delta_length;
	std::uint32_t	au_dts_delta_length;
};

class AuPacketizer
{

	using packet_t = std::vector<std::uint8_t>;
	using queue_t = std::queue<packet_t>;

	au_header_config_t	m_au_header_config;
	queue_t				m_packet_queue;

public:
	AuPacketizer(const au_header_config_t& au_header_config);

	std::size_t PushData(const void* data, std::size_t size);
	std::size_t PopData(void* data = nullptr, std::size_t size = 0);
	bool DropData();

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
