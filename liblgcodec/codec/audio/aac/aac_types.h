#ifndef AAC_TYPES_H
#define AAC_TYPES_H

#include <cstdint>

namespace largo
{

namespace codec
{

namespace audio
{

enum class aac_profile_id_t
{
	aac_profile_unknown,
	aac_profile_ld,
	aac_profile_eld
};

const std::uint32_t default_au_size_length = 13;
const std::uint32_t default_au_index_length = 3;
const std::uint32_t default_au_index_delta_length = default_au_index_length;

struct au_header_options_t
{
	std::uint32_t au_size_length;
	std::uint32_t au_index_length;
	std::uint32_t au_index_delta_length;
	std::uint32_t au_cts_delta_length;
	std::uint32_t au_dts_delta_length;
};

const au_header_options_t default_au_header_options= { default_au_size_length, default_au_index_length, default_au_index_delta_length };

} // audio

} // codec

} // largo

#endif // AAC_TYPES_H
