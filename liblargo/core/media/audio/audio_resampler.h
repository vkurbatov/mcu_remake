#ifndef AUDIO_RESAMPLER_H
#define AUDIO_RESAMPLER_H

#include "core/media/audio/audio_format.h"

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
	audio_format_t	m_audio_format;

	audio_buffer_t	m_internal_resampler_buffer;

public:

	static const std::uint32_t max_volume = 100u;
	static const std::uint32_t min_volume = 0u;

	// Static methods
public:
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

	AudioResampler(const audio_format_t& audio_format);
    virtual ~AudioResampler() override {}

	// methods with external output resampling buffer

	std::int32_t operator()(const audio_format_t& audio_format, const void* input_data, std::size_t input_size, void* output_data, std::size_t output_size = 0, bool reverse = false) const;
	std::int32_t operator()(const audio_format_t& audio_format, const audio_buffer_t& input_buff, void* output_data, std::size_t output_size = 0, bool reverse = false) const;

	// methods with internal output resampling buffer

	const audio_buffer_t& operator()(const audio_format_t& audio_format, const void* input_data, std::size_t input_size, bool reverse = false);
	const audio_buffer_t& operator()(const audio_format_t& audio_format, const audio_buffer_t& input_buff, bool reverse = false);



	// IAudioFormatter interface
public:

	virtual const audio_format_t& GetAudioFormat() const override;

	virtual bool SetAudioFormat(const audio_format_t& audio_format) override;

};

} // audio

} // media

} // core

#endif // AUDIO_RESAMPLER_H
