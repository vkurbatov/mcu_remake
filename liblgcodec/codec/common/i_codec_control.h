#ifndef I_CODEC_CONTROL_H
#define I_CODEC_CONTROL_H

#include <cstdint>

namespace largo
{

namespace codec
{

class ICodecOptions;

class ICodecControl
{
public:
	virtual ~ICodecControl() {}
	virtual bool SetOptions(const ICodecOptions& options) = 0;
	virtual const ICodecOptions& GetOption() const = 0;
};

}

}

#endif // I_CODEC_CONTROL_H
