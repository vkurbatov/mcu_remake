#include "aac_audio_codec_options.h"
#include <algorithm>

extern "C"
{
#include <libavcodec/avcodec.h>
}

#include "codec/audio/libav/libav_audio_transcoder.h"
#include "common/bit_stream.tpp"

namespace largo
{

namespace codec
{

namespace audio
{

namespace aac_config_utils
{

static inline std::uint32_t get_sample_rate(std::uint32_t index)
{
	const std::uint32_t sample_rate_table[] =
	{
		96000,
		88200,
		64000,
		48000,
		44100,
		32000,
		24000,
		22050,
		16000,
		12000,
		11025,
		8000,
		7350
	};

	return index < 0x0D ? sample_rate_table[index] : 0;
}

static inline std::uint32_t get_sample_rate_index(std::uint32_t sample_rate)
{
	for (auto i = 0; i < 0x0D; i++)
	{
		if (sample_rate == get_sample_rate(i))
		{
			return i;
		}
	}

	return 0x0F;
}

static std::size_t get_config_lenght(std::uint64_t config)
{
	std::size_t result = sizeof(config);

	auto config_data = reinterpret_cast<std::uint8_t*>(&config);

	while (result > 0)
	{
		if (config_data[result - 1] != 0)
		{
			break;
		}
		result--;
	}

	return result;
}

} // aac_config_utils


const option_key_t AacAudioCodecOptions::audio_codec_aac_option_header_size_length = "codec.audio.aac.header.size_lenght";
const option_key_t AacAudioCodecOptions::audio_codec_aac_option_header_index_length = "codec.audio.aac.header.index_delta_lenght";
const option_key_t AacAudioCodecOptions::audio_codec_aac_option_header_index_delta_length = "codec.audio.aac.header.index_delta_lenght";
const option_key_t AacAudioCodecOptions::audio_codec_aac_option_header_cts_delta_length = "codec.audio.aac.header.cts_delay_lenght";
const option_key_t AacAudioCodecOptions::audio_codec_aac_option_header_dts_delta_length = "codec.audio.aac.header.dts_delay_lenght";
const option_key_t AacAudioCodecOptions::audio_codec_aac_option_header_rap_length = "codec.audio.aac.header.rap_lenght";
const option_key_t AacAudioCodecOptions::audio_codec_aac_option_header_stream_state_length = "codec.audio.aac.header.stream_state_lenght";

#define GET_AAC_RULES_OPTION(param) GET_OPTION(audio_codec_aac_option_header_, options, aac_header_rules, param)
#define SET_AAC_RULES_OPTION(param) SET_OPTION(audio_codec_aac_option_header_, options, aac_header_rules, param)

aac_profile_id_t AacAudioCodecOptions::GetAacProfileId(const IOptions &options, const aac_profile_id_t& default_aac_profile_id)
{
	aac_profile_id_t result = default_aac_profile_id;

	std::int32_t av_profile = FF_PROFILE_UNKNOWN;

	if (options.GetOption(LibavAudioTranscoder::libav_audio_codec_option_profile, &av_profile, sizeof(av_profile)))
	{
		switch(av_profile)
		{
			case FF_PROFILE_AAC_LD:
				result = aac_profile_id_t::aac_profile_ld;
			break;

			case FF_PROFILE_AAC_ELD:
				result = aac_profile_id_t::aac_profile_eld;
			break;
		}
	}

	return result;
}

void AacAudioCodecOptions::SetAacProfileId(IOptions &options, const aac_profile_id_t &aac_profile_id)
{
	std::int32_t av_profile = FF_PROFILE_UNKNOWN;

	switch(aac_profile_id)
	{
		case aac_profile_id_t::aac_profile_ld:
			av_profile = FF_PROFILE_AAC_LD;
		break;
		case aac_profile_id_t::aac_profile_eld:
			av_profile = FF_PROFILE_AAC_ELD;
		break;
	}

	options.SetOption(LibavAudioTranscoder::libav_audio_codec_option_profile, &av_profile, sizeof(av_profile));
}

bool AacAudioCodecOptions::GetAacHeaderRules(const IOptions &options, aac_header_rules_t& aac_header_rules)
{
	return
	GET_AAC_RULES_OPTION(size_length) &
	GET_AAC_RULES_OPTION(index_length) &
	GET_AAC_RULES_OPTION(index_delta_length) &
	GET_AAC_RULES_OPTION(cts_delta_length) &
	GET_AAC_RULES_OPTION(dts_delta_length) &
	GET_AAC_RULES_OPTION(rap_length) &
	GET_AAC_RULES_OPTION(stream_state_length);
}

void AacAudioCodecOptions::SetAacHeaderRules(IOptions &options, const aac_header_rules_t &aac_header_rules)
{
	SET_AAC_RULES_OPTION(size_length);
	SET_AAC_RULES_OPTION(index_length);
	SET_AAC_RULES_OPTION(index_delta_length);
	SET_AAC_RULES_OPTION(cts_delta_length);
	SET_AAC_RULES_OPTION(dts_delta_length);
	SET_AAC_RULES_OPTION(rap_length);
	SET_AAC_RULES_OPTION(stream_state_length);
}

std::uint64_t AacAudioCodecOptions::GetAacConfig(const IOptions &options)
{
	std::uint64_t config = 0;

	auto profile = GetAacProfileId(options);
	auto sample_rate = GetSampleRate(options);
	auto channels = GetChannels(options);

	if (profile != aac_profile_id_t::aac_profile_unknown && sample_rate != 0 && channels != 0)
	{
		return EncodeConfig(profile, sample_rate, channels);
	}

	return config;
}

bool AacAudioCodecOptions::SetAacConfig(IOptions &options, uint64_t config)
{
	bool result = false;

	aac_profile_id_t profile_id = aac_profile_id_t::aac_profile_unknown;
	std::uint32_t sample_rate = 0;
	std::uint32_t channels = 0;

	if (DecodeConfig(config, profile_id, sample_rate, channels))
	{
		SetAacProfileId(options, profile_id);
		SetSampleRate(options, sample_rate);
		SetChannels(options, channels);
		options.SetOption(LibavAudioTranscoder::libav_audio_codec_option_extra_data, &config, 4);

		result = true;
	}

	return result;
}

uint64_t AacAudioCodecOptions::EncodeConfig(const aac_profile_id_t &aac_profile_id, uint32_t sample_rate, uint32_t channels)
{

	std::uint64_t config = 0;

	if (aac_profile_id != aac_profile_id_t::aac_profile_unknown && sample_rate != 0 && channels != 0)
	{
		BitStreamWriter writer(&config, true);

		if (aac_profile_id == aac_profile_id_t::aac_profile_ld)
		{
			writer.WriteValue(FF_PROFILE_AAC_LD + 1, 5);
		}
		else
		{
			writer.WriteValue(31, 5);
			writer.WriteValue(7, 6);
		}

		auto sample_rate_index = aac_config_utils::get_sample_rate_index(sample_rate);
		writer.WriteValue(sample_rate_index, 4);

		if (sample_rate_index == 0x0F)
		{
			writer.WriteValue(sample_rate, 24);
		}
		writer.WriteValue(channels, 4);
        writer.WriteValue(int(0), 3);
	}

	return config;
}

bool AacAudioCodecOptions::DecodeConfig(uint64_t config, aac_profile_id_t &aac_profile_id, uint32_t &sample_rate, uint32_t &channels)
{
	bool result = false;

	BitStreamReader reader(&config, true);

	auto profile = reader.ReadValue<std::int32_t>(5);

	if (profile == 31)
	{
		profile += reader.ReadValue<std::int32_t>(6) + 1;
	}

	if (profile == FF_PROFILE_AAC_LD + 1 || profile == FF_PROFILE_AAC_ELD + 1)
	{
		aac_profile_id = profile == FF_PROFILE_AAC_LD + 1 ? aac_profile_id_t::aac_profile_ld : aac_profile_id_t::aac_profile_eld;

		auto sample_rate_index = reader.ReadValue<std::int32_t>(4);

		if (sample_rate_index != 0x0F)
		{
			sample_rate = aac_config_utils::get_sample_rate(sample_rate_index);
		}
		else
		{
			sample_rate = reader.ReadValue<std::int32_t>(24);
		}

		if (sample_rate != 0)
		{
			channels = reader.ReadValue<std::uint32_t>(4);

			if (channels != 0)
			{
				result = true;
			}
		}
	}

	return result;
}

AacAudioCodecOptions::AacAudioCodecOptions(const aac_profile_id_t& aac_profile_id, std::uint32_t sample_rate, std::uint32_t channels)
	: AudioCodecOptions(sample_rate, 0, channels, sample_format_t::pcm_16)
{
	SetAacProfileId(aac_profile_id);
	SetAacConfig(EncodeConfig(aac_profile_id, sample_rate, channels));
}

AacAudioCodecOptions::AacAudioCodecOptions(uint64_t config)
	: AudioCodecOptions(48000, 0, 1, sample_format_t::float_32)
{
	SetAacConfig(config);
}

AacAudioCodecOptions::AacAudioCodecOptions(const IOptions &options)
	: AudioCodecOptions(options)
{
	auto config = GetAacConfig(options);

	if (config != 0)
	{
		SetAacConfig(config);
	}

	aac_header_rules_t rules = { };

	if (GetAacHeaderRules(options, rules))
	{
		SetAacHeaderRules(rules);
	}

}

aac_profile_id_t AacAudioCodecOptions::GetAacProfileId(const aac_profile_id_t& default_aac_profile_id) const
{
	return GetAacProfileId(*this, default_aac_profile_id);
}

void AacAudioCodecOptions::SetAacProfileId(const aac_profile_id_t& aac_profile_id)
{
	SetAacProfileId(*this, aac_profile_id);
}

bool AacAudioCodecOptions::GetAacHeaderRules(aac_header_rules_t& aac_header_rules) const
{
	return GetAacHeaderRules(*this, aac_header_rules);
}

void AacAudioCodecOptions::SetAacHeaderRules(const aac_header_rules_t &aac_header_rules)
{
	SetAacHeaderRules(*this, aac_header_rules);
}

std::uint64_t AacAudioCodecOptions::GetAacConfig() const
{
	return GetAacConfig(*this);
}

bool AacAudioCodecOptions::SetAacConfig(std::uint64_t config)
{
	return SetAacConfig(*this, config);
}

} // audio

} // codec

} // largo

