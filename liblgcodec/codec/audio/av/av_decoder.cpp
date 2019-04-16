#include "av_decoder.h"
#include "common/options_helper.h"

namespace largo
{

namespace codec
{

namespace audio
{

namespace av
{

AvDecoder::AvDecoder(std::uint32_t codec_id)
	: m_codec_id(codec_id)
	, m_is_init(false)
{

}

} // av

} // audio

} // codec

} // largo
