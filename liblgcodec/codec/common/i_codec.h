#ifndef I_CODEC
#define I_CODEC

#include "codec/common/i_codec_module.h"
#include "codec/common/i_codec_control.h"
#include "codec/common/i_codec_transcoder.h"

namespace largo
{

namespace codec
{

enum class codec_type_t
{
	codec_encoder,
	codec_decoder
};

class ICodec : virtual public ICodecModule, virtual public ICodecControl, virtual public ICodecTranscoder
{
public:
		virtual ~ICodec() {}
};

}

}

#endif // I_CODEC
