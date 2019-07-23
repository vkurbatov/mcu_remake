#include "audio_codec.h"

namespace largo
{

namespace codec
{

namespace audio
{

AudioCodec::AudioCodec(bool is_encoder, const std::string& codec_family, const IOptions& options)
	: m_audio_codec_options(options)
	, m_codec_family(codec_family)
	, m_is_open(false)
	, m_is_encoder(is_encoder)
{

}

int32_t AudioCodec::operator ()(const void* input_data, std::size_t input_size, void* output_data, std::size_t output_size)
{
	return IsOpen() ? internal_transcode(input_data, input_size, output_data, output_size) : -EINVAL;
}

transcoder_direction_t AudioCodec::GetTranscodeDirection() const
{
	return m_is_encoder ? transcoder_direction_t::encoder_direction : transcoder_direction_t::decoder_direction;
}

bool AudioCodec::SetOptions(const IOptions& options)
{

	bool need_open = m_is_open;

	m_audio_codec_options << options;

	if (need_open)
	{
		internal_close();
	}

	bool result = internal_reconfigure(m_audio_codec_options);

	if (result && need_open)
	{
		m_is_open = internal_open();
	}

	return result;
}

const IOptions& AudioCodec::GetOptions() const
{
	return m_audio_codec_options;
}

const IOptions& AudioCodec::NormalizeOptions(IOptions& options) const
{
	return options;
}

const IOptions& AudioCodec::NormalizeOptions()
{
	return NormalizeOptions(m_audio_codec_options);
}

bool AudioCodec::Open()
{
	if (m_is_open)
	{
		internal_close();
	}

	m_is_open = internal_open();

	return m_is_open;
}

bool AudioCodec::Close()
{
	if (m_is_open)
	{
		internal_close();
	}

	m_is_open = false;

	return true;
}

bool AudioCodec::IsOpen() const
{
	return m_is_open;
}

const std::string& AudioCodec::GetCodecFamily() const
{
	return m_codec_family;
}

const std::string& AudioCodec::GetCodecName() const
{
	return m_codec_family;
}

codec_media_type_t AudioCodec::GetCodecMediaType() const
{
	return codec_media_type_t::audio_codec;
}

AudioCodecOptions& AudioCodec::get_audio_codec_options()
{
	return m_audio_codec_options;
}


} // audio

} // codec

} // largo
