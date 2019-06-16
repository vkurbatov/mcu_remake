#ifndef AAC_TYPES_H
#define AAC_TYPES_H

#include "codec/audio/aac/au_types.h"

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

using aac_header_rules_t = au_header_rules_t;

const aac_header_rules_t default_aac_header_rules = default_au_header_rules;

} // audio

} // codec

} // largo

#endif // AAC_TYPES_H
