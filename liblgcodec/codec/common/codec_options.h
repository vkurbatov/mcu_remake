#ifndef CODEC_OPTIONS_H
#define CODEC_OPTIONS_H

#include "i_codec_options.h"
#include "common/options.h"

namespace largo
{

namespace codec
{

class CodecOptions : virtual public ICodecOptions, virtual public Options
{
public:
	CodecOptions();
};

}

}

#endif // CODEC_OPTIONS_H
