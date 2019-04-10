#ifndef I_CODEC_H
#define I_CODEC_H

#include <cstdint>

namespace codec
{

class ICodecOptions;

enum class codec_media_type_t
{
	unknown_codec = -1,
	audio_codec,
	video_codec,
	application_codec
};

class ICodec
{
public:
	virtual bool Open() = 0;
	virtual bool Close() = 0;
	virtual bool IsOpen() const = 0;
	virtual codec_media_type_t& GetCodecMediaType() const;
};

class ICodecControl
{
public:
	virtual bool SetOptions(const ICodecOptions& options) = 0;
	virtual const ICodecOptions& GetOption() const = 0;
};

class ITranscoder
{
public:
	virtual std::size_t Transcode(const void* input_data
							   , std::size_t input_size
							   , void* output_data
							   , std::size_t output_size);
};



//class IEncoder

}

#endif
