#ifndef AU_BUILDER_H
#define AU_BUILDER_H

#include "codec/audio/aac/au_types.h"

namespace largo
{

namespace codec
{

namespace audio
{

class AuBuilder
{

	au_header_config_t	m_au_header_config;


public:
	AuBuilder(const au_header_config_t& au_header_config);

private:


};

} // audio

} // codec

} // largo

#endif // AU_BUILDER_H
