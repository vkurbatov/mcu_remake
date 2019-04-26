#ifndef AV_CODEC_H
#define AV_CODEC_H

#include "codec/audio/audio_codec_types.h"

#include <cstdint>
#include <memory>
#include <vector>



namespace largo
{

namespace codec
{

namespace audio
{

struct libav_context_t;

const std::uint32_t default_libav_sample_rate = 8000;
const sample_format_t default_libav_sample_format = sample_format_t::pcm_16;
const std::uint32_t default_libav_channels = 1;
const std::uint32_t default_libav_bit_rate = 0;
const std::uint32_t default_libav_frame_size = 0;
const std::int32_t default_libav_profile = -99;

struct libav_codec_config_t
{
	std::uint32_t				sample_rate;
	sample_format_t				sample_format;
	std::uint32_t				channels;
	std::uint32_t				bit_rate;
	std::uint32_t				frame_size;
	std::int32_t				profile;

};

const libav_codec_config_t default_libav_codec_config = { default_libav_sample_rate
														  , default_libav_sample_format
														  , default_libav_channels
														  , default_libav_bit_rate
														  , default_libav_frame_size
														  , default_libav_profile
														};

class LibavWrapper
{
	audio_codec_id_t											m_codec_id;
	libav_codec_config_t										m_codec_config;
	std::unique_ptr<libav_context_t,void(*)(libav_context_t*)>	m_context;

	bool														m_is_encoder;
	bool														m_is_open;

	std::vector<std::uint8_t>									m_buffer;


public:
	LibavWrapper(audio_codec_id_t codec_id
			, bool is_encoder
			, const libav_codec_config_t& config);
	~LibavWrapper();

	bool Open();
	bool Close();
	bool IsOpen() const;

	bool IsEncoder() const;

	std::int32_t Transcode(const void* input_data
						   , std::size_t input_size
						   , void* output_data
						   , std::size_t output_size);

	void SetConfig(const libav_codec_config_t& config);
	const libav_codec_config_t& GetConfig() const;
	audio_codec_id_t GetCodecId() const;

};

using libav_wrapper_ptr = std::unique_ptr<LibavWrapper>;

} // audio

} // codec

} // largo

#endif // AV_CODEC_H
