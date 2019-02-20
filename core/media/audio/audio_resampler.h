#ifndef AUDIO_RESAMPLER_H
#define AUDIO_RESAMPLER_H

#include "media/audio/audio_format.h"

#include "i_audio_formatter.h"

#include <vector>

namespace core
{

namespace media
{

namespace audio
{

using audio_buffer_t = std::vector<std::uint8_t>;

class AudioResampler : public IAudioFormatter
{
	audio_format_t	m_input_format;
	audio_format_t	m_output_format;

	audio_buffer_t	m_internal_resampler_buffer;

public:

	static const std::uint32_t max_volume = 100u;
	static const std::uint32_t min_volume = 0u;

	static std::int32_t Resampling(const audio_format_t& input_format
								   , const audio_format_t& output_format
								   , const void* input_data, std::size_t input_size
								   , void* output_data, std::size_t output_size = 0);
	static std::int32_t Resampling(const audio_format_t& input_format
								   , const audio_format_t& output_format
								   , const void* input_data, std::size_t input_size
								   , audio_buffer_t& output_buffer);

	static std::int32_t Resampling(const audio_format_t& input_format
								   , const audio_format_t& output_format
								   , const audio_buffer_t& input_buffer
								   , audio_buffer_t& output_buffer);

	static std::int32_t Resampling(const audio_format_t& input_format
								   , const audio_format_t& output_format
								   , const audio_buffer_t& input_buffer
								   , void* output_data, std::size_t output_size = 0);

	AudioResampler(const audio_format_t& input_format, const audio_format_t& output_format);
	virtual ~AudioResampler() override = default;

	// methods with external output resampling buffer

	std::int32_t operator()(const void* input_data, std::size_t input_size, void* output_data, std::size_t output_size = 0) const;
	std::int32_t operator()(const void* input_data, std::size_t input_size, const audio_format_t& input_format, void* output_data, std::size_t output_size = 0) const;

	std::int32_t operator()(const audio_buffer_t& input_buff, void* output_data, std::size_t output_size = 0) const;
	std::int32_t operator()(const audio_buffer_t& input_buff, const audio_format_t& input_format, void* output_data, std::size_t output_size = 0) const;

	// methods with internal output resampling buffer

	const audio_buffer_t& operator()(const void* input_data, std::size_t input_size, const audio_format_t& input_format);
	const audio_buffer_t& operator()(const audio_buffer_t& input_buff, const audio_format_t& input_format);

	const audio_buffer_t& operator()(const void* input_data, std::size_t input_size);
	const audio_buffer_t& operator()(const audio_buffer_t& input_buff);

	// IAudioFormatter interface
public:

	virtual const audio_format_t&GetInputFormat() const override;
	virtual const audio_format_t&GetOutputFormat() const override;

	virtual void SetInputFormat(const audio_format_t& input_format) override;
	virtual void SetOutputFormat(const audio_format_t& output_format) override;

};

} // audio

} // media

} // core

#endif // AUDIO_RESAMPLER_H
