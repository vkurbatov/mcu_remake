#ifndef AV_CODEC_H
#define AV_CODEC_H


#include "codec/common/i_codec.h"
#include "codec/audio/audio_codec_options.h"

#include <memory>

extern "C"
{
struct AVCodec;
struct AVCodecContext;
struct AVFrame;
}

namespace largo
{

namespace codec
{

namespace audio
{

namespace av
{

struct av_context_t
{
	AVCodec*			av_codec;
	AVCodecContext*		av_codec_context;
	AVFrame*			av_frame;
	std::uint32_t		codec_id;
	std::uint32_t		profile;
};

class AvCodec : virtual public ICodec
{
	av_context_t		m_av_context;

	Options				m_options;
	bool				m_is_init;

public:
	AvCodec(std::uint32_t codec_id);

	// ICodecControl interface
public:
	bool SetOptions(const IOptions& options) override;
	const IOptions& GetOption() const override;

	// ICodecModule interface
public:
	bool Open() override;
	bool Close() override;
	bool IsOpen() const override;
	bool IsInit() const override;
	codec_media_type_t GetCodecMediaType() const override;

protected:

	virtual std::int32_t internal_open(const audio_codec_options_t& audio_options, av_context_t& av_context) = 0;
	virtual std::int32_t internal_close() = 0;
};

} // av

} // audio

} // codec

} // largo


#endif // AV_CODEC_H
