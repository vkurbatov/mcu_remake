#include "au_builder.h"

/*
std::size_t au_header_config_t::au_header_length(bool first_frame) const
{
		return size_length
						+ first_frame ? index_length : index_delta_length
						+ cts_delta_length > 0 ? cts_delta_length + 1 : 0
						+ dts_delta_length > 0 ? dts_delta_length + 1 : 0
						+ rap_length + stream_state_length;
}
*/

namespace largo
{

namespace codec
{

namespace audio
{

AuBuilder::AuBuilder()
{

}

} // audio

} // codec

} // largo
