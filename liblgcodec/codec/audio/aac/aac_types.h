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

struct aac_packetize_options_t
{
	std::uint32_t au_size;
	std::uint32_t au_index;
	std::uint32_t au_index_delta;
};

} // audio

} // codec

} // largo

#endif // AAC_TYPES_H
