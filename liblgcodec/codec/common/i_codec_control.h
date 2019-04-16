#ifndef I_CODEC_CONTROL_H
#define I_CODEC_CONTROL_H

#include <cstdint>

namespace largo
{

class IOptions;

namespace codec
{

class ICodecControl
{
public:
	virtual ~ICodecControl() {}
	virtual bool SetOptions(const IOptions& options) = 0;
	virtual const IOptions& GetOption() const = 0;
};

}

}

#endif // I_CODEC_CONTROL_H
