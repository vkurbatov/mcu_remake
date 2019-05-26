#ifndef AU_PACKET_H
#define AU_PACKET_H

#include <cstdint>

namespace largo
{

namespace codec
{

namespace audio
{

struct au_header
{
	std::uint32_t	size;
	std::uint32_t	index;
	bool			cts_flag;
	std::uint32_t	cts_delta;
	bool			dts_flag;
	std::uint32_t	dts_delta;
	std::uint32_t	rap_flag;
};

struct au_header_config
{
	std::uint32_t	au_size_length;
	std::uint32_t	au_index_length;
	std::uint32_t	au_index_delta_length;
	std::uint32_t	au_cts_delta_length;
	std::uint32_t	au_dts_delta_length;
};


class AuPacket
{

public:
	AuPacket();
};

} // audio

} // codec

} // largo

#endif // AU_PACKET_H
