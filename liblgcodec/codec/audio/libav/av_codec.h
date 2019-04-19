#ifndef AV_CODEC_H
#define AV_CODEC_H

#include <cstdint>
#include <memory>
#include <vector>

namespace largo
{

namespace codec
{

namespace audio
{

namespace libav
{

struct libav_context_t;

enum class libav_codec_id_t
{
	codec_unknown = -1,
	codec_g723_1,
	codec_aac_ld
};

enum class sample_format_t
{
	unknown,
	pcm_8,
	pcm_16,
	pcm_32,
	float_32,
	float_64
};

struct libav_codec_config_t
{
	std::uint32_t				sample_rate;
	sample_format_t				sample_format;
//	std::uint32_t				channels;
	std::uint32_t				bit_rate;
	std::uint32_t				frame_size;
	std::uint64_t				profile;
};

class AvCodec
{
	libav_codec_id_t					m_codec_id;
	libav_codec_config_t				m_codec_config;
	libav_context_t*					m_context;

	bool								m_is_encoder;
	bool								m_is_open;

	std::vector<std::uint8_t>			m_buffer;


public:
	AvCodec(libav_codec_id_t codec_id
			, const libav_codec_config_t& config
			, bool is_encoder);
	~AvCodec();

	bool Open();
	bool Close();
	bool IsOpen() const;

	bool IsEncoder() const;

	std::int32_t Transcode(const void* input_data
						   , std::size_t input_size
						   , void* output_data
						   , std::size_t output_size);

	const libav_codec_config_t& GetConfig() const;
	libav_codec_id_t GetCodecId() const;

};

} // libav

} // audio

} // codec

} // largo

#endif // AV_CODEC_H
