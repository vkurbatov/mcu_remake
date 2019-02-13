#ifndef AUDIO_RESAMPLER_H
#define AUDIO_RESAMPLER_H

#include "audio_formatter.h"

#include <vector>

namespace core
{

namespace media
{

namespace audio
{

using audio_buffer_t = std::vector<std::uint8_t>;

class AudioResampler : public AudioFormatter
{

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

    AudioResampler(const audio_format_t& input_format, const audio_format_t& output_format);

    std::int32_t Resampling(const void* input_data, std::size_t input_size, void* output_data, std::size_t output_size = 0);
    std::int32_t Resampling(const void* input_data, std::size_t input_size, const audio_format_t& input_fromat, void* output_data, std::size_t output_size = 0);

};

} // audio

} // media

} // core

#endif // AUDIO_RESAMPLER_H
