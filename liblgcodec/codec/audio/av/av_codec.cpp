#include "av_codec.h"
#include "common/options_helper.h"

extern "C"
{
#include <libavcodec/avcodec.h>
}

namespace largo
{

namespace codec
{

namespace audio
{

namespace av
{

AvCodec::AvCodec(std::uint32_t codec_id)
	: m_av_context({nullptr, nullptr, nullptr, codec_id, 0})
{

}

bool AvCodec::SetOptions(const IOptions& options)
{
	m_options.MergeFrom(options);
	m_is_init = true;
}

const IOptions& AvCodec::GetOption() const
{
	return m_options;
}

bool AvCodec::Open()
{
	bool result = false;

	if (IsInit())
	{

		if (!IsOpen() || Close())
		{

			OptionsHelper	options_helper(m_options);

			audio_codec_options_t	audio_codec_options;

			audio_codec_options.codec_options.sample_rate = options_helper.GetOption<std::uint32_t>(CodecOptions::codec_option_sample_rate, 8000);
			audio_codec_options.codec_options.bit_rate = options_helper.GetOption<std::uint32_t>(CodecOptions::codec_option_bit_rate, 0);
			audio_codec_options.format = options_helper.GetOption<sample_fromat_t>(AudioCodecOptions::audio_codec_option_format, sample_fromat_t::pcm16);
			audio_codec_options.num_channels = options_helper.GetOption<std::uint32_t>(AudioCodecOptions::audio_codec_option_num_channels, 1);

			result = internal_open(audio_codec_options, m_av_context) >= 0;

		}
	}

	return result;
}

bool AvCodec::Close()
{
	bool result = false;

	if (m_av_context.av_codec != nullptr)
	{

	}

	return result;
}

bool AvCodec::IsOpen() const
{
	return m_av_context.av_codec != nullptr
			&& m_av_context.av_codec_context != nullptr
			&& m_av_context.av_frame != nullptr;
}

bool AvCodec::IsInit() const
{
	return m_is_init;
}

codec_media_type_t AvCodec::GetCodecMediaType() const
{
	return codec_media_type_t::audio_codec;
}

} // av

} // audio

} // codec

} // largo
