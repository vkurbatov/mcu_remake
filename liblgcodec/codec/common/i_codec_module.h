#ifndef I_CODEC_MODULE_H
#define I_CODEC_MODULE_H

#include <cstdint>

namespace largo
{

namespace codec
{

enum class codec_media_type_t
{
	unknown_codec = -1,
	audio_codec,
	video_codec,
	application_codec
};

class ICodecModule
{
public:
	virtual ~ICodecModule() {}

	virtual bool Open() = 0;
	virtual bool Close() = 0;

	virtual bool IsOpen() const = 0;
	virtual bool IsInit() const = 0;

	virtual codec_media_type_t GetCodecMediaType() const = 0;
};

}

}

#endif // I_CODEC_MODULE_H
