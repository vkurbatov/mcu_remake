#ifndef AUDIO_FORMATTER_H
#define AUDIO_FORMATTER_H

#include "audio_format.h"

namespace core
{

namespace media
{

namespace audio
{

class AudioFormatter
{

    audio_format_t m_input_format;
    audio_format_t m_output_format;

public:

    AudioFormatter(const audio_format_t& input_format = null_audio_format, const audio_format_t& output_format = null_audio_format);
    virtual ~AudioFormatter() = default;

    virtual const audio_format_t& GetInputFormat() const;
    virtual const audio_format_t& GetOutputFormat() const;

    virtual void SetInputFormat(const audio_format_t& input_fromat);
    virtual void SetOutputFormat(const audio_format_t& output_fromat);

};

} // audio

} // media

} // core


#endif // AUDIO_FORMATTER_H
