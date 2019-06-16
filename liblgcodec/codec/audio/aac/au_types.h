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
	bool			cts_flag;
	std::uint32_t	cts_delta;
	bool			dts_flag;
	std::uint32_t	dts_delta;
	std::uint32_t	rap_flag;
	std::uint32_t	stream_state;

	bool is_valid() const { return size > 0; }
};

const std::uint32_t au_header_section_size_length = 16;
const std::uint32_t default_au_header_size_length = 13;
const std::uint32_t default_au_header_index_length = 3;
const std::uint32_t default_au_header_index_delta_length = default_au_header_index_length;
const std::uint32_t default_au_header_cts_delta_length = 0;
const std::uint32_t default_au_header_dts_delta_length = 0;
const std::uint32_t default_au_header_rap_length = 0;
const std::uint32_t default_au_header_stream_state_length = 0;

struct au_header_rules_t
{
	std::uint32_t	size_length;
	std::uint32_t	index_length;
	std::uint32_t	index_delta_length;
	std::uint32_t	cts_delta_length;
	std::uint32_t	dts_delta_length;
	std::uint32_t	rap_length;
	std::uint32_t	stream_state_length;

	std::uint32_t max_data_size() const { return (1 << size_length); }
	bool is_valid_size(std::uint32_t size) const { return size > 0 && size < max_data_size(); }
};

const au_header_rules_t default_au_header_rules = {
	default_au_header_size_length
	, default_au_header_index_length
	, default_au_header_index_delta_length
	, default_au_header_cts_delta_length
	, default_au_header_dts_delta_length
	, default_au_header_rap_length
	, default_au_header_stream_state_length
};


} // audio

} // codec

} // largo

#endif // AU_TYPES_H
