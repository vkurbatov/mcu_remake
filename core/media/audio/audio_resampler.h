#ifndef AUDIO_RESAMPLER_H
#define AUDIO_RESAMPLER_H

#include "audio_format.h"

namespace core
{

namespace media
{

namespace audio
{

struct audio_format_t;

class AudioResampler
{
    audio_format_t m_input_fromat;
    audio_format_t m_output_fromat;

public:
    AudioResampler();
    static std::int32_t Resampling(const audio_format_t& input_format
                          , const audio_format_t& output_format
                          , const void* input_data, std::size_t input_size
                          , void* output_data, std::size_t output_size = 0);

    inline const audio_format_t& GetInputFormat() const { return m_input_fromat; }
    inline void SetInputFormat(const audio_format_t& input_fromat) { m_input_fromat = input_fromat; }

    inline const audio_format_t& GetOutputFormat() const { return m_input_fromat; }
    inline void SetOutputFormat(const audio_format_t& output_fromat) { m_output_fromat = output_fromat; }

    std::int32_t Resampling(const void* input_data, std::size_t input_size, void* output_data, std::size_t output_size = 0);

};

} // audio

} // media

} // core

#endif // AUDIO_RESAMPLER_H
