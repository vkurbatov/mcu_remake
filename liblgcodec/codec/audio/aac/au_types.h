#ifndef AU_TYPES_H
#define AU_TYPES_H

#include <cstdint>

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
	std::uint32_t	cts_delta;
	std::uint32_t	dts_delta;
	std::uint32_t	rap_flag;
	std::uint32_t	stream_state;

	bool is_cts_set() { return cts_delta > 0; }
	bool is_dts_set() { return dts_delta > 0; }
};

struct au_header_config_t
{
	std::uint32_t	size_length;
	std::uint32_t	index_length;
	std::uint32_t	index_delta_length;
	std::uint32_t	cts_delta_length;
	std::uint32_t	dts_delta_length;
	std::uint32_t	rap_length;
	std::uint32_t	stream_state_length;
};


} // audio

} // codec

} // largo

#endif // AU_TYPES_H
