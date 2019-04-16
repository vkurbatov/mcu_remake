#ifndef AV_DECODER_H
#define AV_DECODER_H

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

class AvDecoder : virtual public ICodec
{
	std::uint32_t		m_codec_id;

	AVCodec*			m_av_codec;
	AVCodecContext*		m_av_context;
	AVFrame*			m_av_frame;

	Options				m_options;
	bool				m_is_init;

public:
	AvDecoder(std::uint32_t codec_id);

	// ICodecTranscoder interface
public:
	std::int32_t operator ()(const void* input_data, std::size_t input_size, void* output_data, std::size_t output_size) override;
	transcoder_mode_t GetTranscoderMode() const override;

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
};

} // av

} // audio

} // codec

} // largo

#endif // AV_DECODER_H
