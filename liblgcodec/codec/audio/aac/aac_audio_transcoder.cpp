#include "aac_audio_transcoder.h"
#include "aac_audio_codec_options.h"

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

namespace aac_utils
{


} // aac_utils

AacAudioTranscoder::AacAudioTranscoder(bool is_encoder, const aac_profile_id_t& aac_profile, std::uint32_t sample_rate)
	: LibavAudioTranscoder(audio_codec_id_t::audio_codec_aac, is_encoder, AacAudioCodecOptions(aac_profile, sample_rate))
	, m_aac_packetizer(is_encoder)
{

}

AacAudioTranscoder::AacAudioTranscoder(bool is_encoder, AudioCodecOptions &audio_codec_options)
	: LibavAudioTranscoder(audio_codec_id_t::audio_codec_aac, is_encoder, AacAudioCodecOptions(audio_codec_options))
	, m_aac_packetizer(is_encoder)
{
	SetOptions(audio_codec_options);
}

bool AacAudioTranscoder::internal_reconfigure(AudioCodecOptions& audio_codec_options)
{
	auto result = LibavAudioTranscoder::internal_reconfigure(audio_codec_options);

	if (result > 0)
	{
		auto aac_header_rules = m_aac_packetizer.GetAacHeaderRules();

		AacAudioCodecOptions::GetAacHeaderRules(audio_codec_options, aac_header_rules);

		m_aac_packetizer.SetAacHeaderRules(aac_header_rules);
	}

	return result;
}

std::int32_t AacAudioTranscoder::internal_transcode(const void *input_data, std::size_t input_size, void *output_data, std::size_t output_size)
{
	std::int32_t result = 0;

	bool is_encoder = GetTranscodeDirection() == transcoder_direction_t::encoder_direction;

	if (is_encoder)
	{

		prepare_buffer(output_size);

		auto transcode_result = LibavAudioTranscoder::internal_transcode(input_data, input_size, m_transcode_buffer.data(), m_transcode_buffer.size());

		if (transcode_result > 0)
		{
			if (m_aac_packetizer.Push(m_transcode_buffer.data(), transcode_result))
			{
				result = m_aac_packetizer.Pop(output_data, output_size);
			}
		}

	}
	else
	{
		auto depacketize_size  = m_aac_packetizer.Push(input_data, input_size);

		if (depacketize_size > 0)
		{
			prepare_buffer(output_size);

			auto frame_size = m_aac_packetizer.Pop(m_transcode_buffer.data(), output_size);

			if (frame_size > 0 && frame_size <= output_size)
			{
				result = LibavAudioTranscoder::internal_transcode(m_transcode_buffer.data(), frame_size, output_data, output_size);
			}
		}
	}

	return result;
}

void AacAudioTranscoder::prepare_buffer(std::size_t size)
{
	if (m_transcode_buffer.size() < size)
	{
		m_transcode_buffer.resize(size);
	}
}


} // audio

} // codec

} // largo
