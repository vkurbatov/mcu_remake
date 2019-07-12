#include "libav_audio_transcoder.h"
#include "codec/audio/audio_codec_utils.h"

namespace largo
{

namespace codec
{

namespace audio
{

namespace libav_utils
{

bool load_libav_config(libav_codec_config_t& libav_config, const IOptions& options)
{
	return
		options.GetOption(AudioCodecOptions::audio_codec_option_sample_rate, &libav_config.sample_rate, sizeof(libav_config.sample_rate)) &
		options.GetOption(AudioCodecOptions::audio_codec_option_bit_rate, &libav_config.bit_rate, sizeof(libav_config.bit_rate)) &
		options.GetOption(AudioCodecOptions::audio_codec_option_format, &libav_config.sample_format, sizeof(libav_config.sample_format)) &
		options.GetOption(AudioCodecOptions::audio_codec_option_num_channels, &libav_config.channels, sizeof(libav_config.channels)) &
		options.GetOption(LibavAudioTranscoder::libav_audio_codec_option_frame_size, &libav_config.frame_size, sizeof(libav_config.frame_size)) &
		options.GetOption(LibavAudioTranscoder::libav_audio_codec_option_profile, &libav_config.profile, sizeof(libav_config.profile));
}

void store_libav_config(const libav_codec_config_t& libav_config, IOptions& options)
{
	options.SetOption(AudioCodecOptions::audio_codec_option_sample_rate, &libav_config.sample_rate, sizeof(libav_config.sample_rate));
	options.SetOption(AudioCodecOptions::audio_codec_option_bit_rate, &libav_config.bit_rate, sizeof(libav_config.bit_rate));
	options.SetOption(AudioCodecOptions::audio_codec_option_format, &libav_config.sample_format, sizeof(libav_config.sample_format));
	options.SetOption(AudioCodecOptions::audio_codec_option_num_channels, &libav_config.channels, sizeof(libav_config.channels));
	options.SetOption(LibavAudioTranscoder::libav_audio_codec_option_frame_size, &libav_config.frame_size, sizeof(libav_config.frame_size));
	options.SetOption(LibavAudioTranscoder::libav_audio_codec_option_profile, &libav_config.profile, sizeof(libav_config.profile));
}

} // libav_utils

const option_key_t LibavAudioTranscoder::libav_audio_codec_option_frame_size = "codec.audio.libav.frame_size";
const option_key_t LibavAudioTranscoder::libav_audio_codec_option_profile = "codec.audio.libav.profile";

LibavAudioTranscoder::LibavAudioTranscoder(audio_codec_id_t codec_id, bool is_encoder, const IOptions& options)
	: AudioCodec(is_encoder, get_codec_name_from_id(codec_id), options)
	, m_codec_id(codec_id)
	, m_av_codec(create_libav_wrapper(codec_id, is_encoder, options))
{
	NormalizeOptions(get_audio_codec_options());
}

LibavAudioTranscoder::~LibavAudioTranscoder()
{

}

const IOptions& LibavAudioTranscoder::NormalizeOptions(IOptions& options) const
{
	if (m_av_codec != nullptr)
	{
		libav_utils::store_libav_config(m_av_codec->GetConfig(), options);
	}
	return options;
}

bool LibavAudioTranscoder::internal_open()
{
	return m_av_codec != nullptr ? m_av_codec->Open() : true;
}

bool LibavAudioTranscoder::internal_close()
{
	return m_av_codec != nullptr ? m_av_codec->Close() : true;
}

bool LibavAudioTranscoder::internal_reconfigure(AudioCodecOptions& audio_codec_options)
{
	bool result = false;

	if (m_av_codec != nullptr)
	{

		libav_codec_config_t libav_config = m_av_codec->GetConfig();

		libav_utils::load_libav_config(libav_config, audio_codec_options);

		NormalizeOptions(audio_codec_options);
	}

	return result;
}

int32_t LibavAudioTranscoder::internal_transcode(const void* input_data, std::size_t input_size, void* output_data, std::size_t output_size)
{
	std::int32_t result = -EINVAL;

	if (m_av_codec != nullptr)
	{
		result = m_av_codec->Transcode(input_data, input_size, output_data, output_size);
	}

	return result;
}

audio_codec_id_t LibavAudioTranscoder::GetCodecId() const
{
	return m_codec_id;
}

LibavWrapper* LibavAudioTranscoder::create_libav_wrapper(audio_codec_id_t codec_id, bool is_encoder, const IOptions& options)
{
	LibavWrapper* libav_wrapper = nullptr;

	switch (codec_id)
	{
		case audio_codec_id_t::audio_codec_g723_1:
		case audio_codec_id_t::audio_codec_aac:
		{

			libav_codec_config_t libav_codec_config = default_libav_codec_config;

			libav_utils::load_libav_config(libav_codec_config, options);

			libav_wrapper = new LibavWrapper(codec_id, is_encoder, libav_codec_config);
		}
		break;

	}

	return libav_wrapper;
}

} // audio

} // codec

} // largo
