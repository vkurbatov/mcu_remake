#ifndef AUDIO_CODEC_H
#define AUDIO_CODEC_H

#include "codec/audio/i_audio_codec.h"
#include "codec/audio/audio_codec_options.h"

namespace largo
{

namespace codec
{

namespace audio
{

class AudioCodec : virtual public IAudioCodec
{
	AudioCodecOptions		m_audio_codec_options;
	std::string				m_codec_family;

	bool					m_is_open;
	bool					m_is_encoder;

public:
	AudioCodec(bool is_encoder, const std::string& codec_family, const IOptions& options);
	virtual ~AudioCodec() {}

	// ICodecTranscoder interface
public:
	int32_t operator ()(const void* input_data, std::size_t input_size, void* output_data, std::size_t output_size) override;
	transcoder_direction_t GetTranscodeDirection() const override;

	// ICodecControl interface
public:
	bool SetOptions(const IOptions& options) override;
	const IOptions& GetOptions() const override;
	const IOptions& NormalizeOptions(IOptions& options) const override;
	const IOptions& NormalizeOptions() override;

	// ICodecModule interface
public:
	bool Open() override;
	bool Close() override;
	bool IsOpen() const override;

	virtual const std::string& GetCodecFamily() const override;
	virtual const std::string& GetCodecName() const override;

	codec_media_type_t GetCodecMediaType() const override;

protected:

	virtual bool internal_open() = 0;
	virtual bool internal_close() = 0;
	virtual bool internal_reconfigure(AudioCodecOptions& audio_codec_options) = 0;
	virtual std::int32_t internal_transcode(const void* input_data, std::size_t input_size, void* output_data, std::size_t output_size) = 0;

protected:
	virtual AudioCodecOptions& get_audio_codec_options();


};

} // audio

} // codec

} // largo

#endif // AUDIO_CODEC_H
