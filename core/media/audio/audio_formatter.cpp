#include "audio_formatter.h"


namespace core
{

namespace media
{

namespace audio
{

AudioFormatter::AudioFormatter(const audio_format_t& input_format, const audio_format_t& output_format)
	: m_input_format(input_format)
	, m_output_format(output_format)
{

}

const audio_format_t& AudioFormatter::GetInputFormat() const
{
	return m_input_format;
}

const audio_format_t& AudioFormatter::GetOutputFormat() const
{
	return m_output_format;
}

void AudioFormatter::SetInputFormat(const audio_format_t& input_fromat)
{
	m_input_format = input_fromat;
}

void AudioFormatter::SetOutputFormat(const audio_format_t& output_fromat)
{
	m_output_format = output_fromat;
}

} // audio

} // media

} // core
