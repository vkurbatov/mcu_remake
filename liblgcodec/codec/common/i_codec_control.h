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
	virtual const IOptions& GetOptions() const = 0;
	virtual const IOptions& NormalizeOptions(IOptions& options) const = 0;
	virtual const IOptions& NormalizeOptions() = 0;
};

}

}

#endif // I_CODEC_CONTROL_H
