#ifndef I_CODEC_OPTIONS_H
#define I_CODEC_OPTIONS_H

#include "common/i_options.h"

namespace largo
{

namespace codec
{

class ICodecOptions: virtual public IOptions
{
public:
    virtual ~ICodecOptions() {}
};

}

}

#endif // I_CODEC_OPTIONS_H
