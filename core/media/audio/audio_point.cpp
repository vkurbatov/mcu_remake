#include "audio_point.h"
#include "errno.h"

#include "audio_resampler.h"

#include <core-tools/logging.h>

namespace core
{

namespace media
{

namespace audio
{

AudioPoint::AudioPoint(const audio_format_t& input_format, const audio_format_t& output_format) :
	m_audio_resampler(input_format, output_format)
{

}

std::int32_t AudioPoint::Write(const void* data, std::size_t size, const audio_format_t& audio_format, std::uint32_t volume, std::uint32_t flags)
{
    std::int32_t result = -EINVAL;

    if (data != nullptr && size > 0)
    {
		if (audio_format == GetOutputFormat())
		{
			result = Write(data, size, flags);
		}
		else
		{
			result = 0;

			auto& resample_buffer = m_audio_resampler(data, size, audio_format);

			if (resample_buffer.size() > 0)
			{
				result = Write(resample_buffer.data(), resample_buffer.size(), flags);
			}
		}
    }
    else
    {
		LOG(error) << "Failed write audio frame into audio point - invalid argument: data = " << data << ", size = " << size LOG_END;
    }

    return result;
}

std::int32_t AudioPoint::Read(void* data, std::size_t size, const audio_format_t& audio_format, std::uint32_t volume, std::uint32_t flags)
{
	std::int32_t result = -EINVAL;

	if (data != nullptr && size > 0)
	{
		if (audio_format == GetInputFormat())
		{
			result = Read(data, size, flags);
		}
		else
		{
			auto input_size = GetInputFormat().octets_from_format(audio_format, size);

			if (input_size > m_output_resampler_buffer.size())
			{
				m_output_resampler_buffer.resize(input_size);
			}

			result = Read(m_output_resampler_buffer.data(), input_size, flags);

			if (result > 0)
			{
				result = AudioResampler::Resampling(GetInputFormat(), audio_format, m_output_resampler_buffer.data(), result, data, size);
			}
		}
	}
	else
	{
		LOG(error) << "Failed read audio frame from audio point - invalid argument: data = " << data << ", size = " << size LOG_END;
	}

	return result;
}

const audio_format_t& AudioPoint::GetInputFormat() const
{
	return m_audio_resampler.GetInputFormat();
}

const audio_format_t& AudioPoint::GetOutputFormat() const
{
	return m_audio_resampler.GetOutputFormat();
}

void AudioPoint::SetInputFormat(const audio_format_t& input_fromat)
{
	m_audio_resampler.SetInputFormat(input_fromat);
}

void AudioPoint::SetOutputFormat(const audio_format_t& output_fromat)
{
	m_audio_resampler.SetOutputFormat(output_fromat);
}

} // audio

} // media

} // core
