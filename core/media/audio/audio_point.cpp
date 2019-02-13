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

AudioPoint::AudioPoint(const audio_format_t& input_format, const audio_format_t& output_format)
	: AudioFormatter(input_format, output_format)
{

}

std::int32_t AudioPoint::Write(const void* data, std::size_t size, const audio_format_t& audio_format, std::uint32_t flags)
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

			auto output_size = AudioResampler::Resampling(audio_format, GetOutputFormat(), data, size, m_output_resample_buffer);

			if (output_size > 0)
			{
				result = Write(m_output_resample_buffer.data(), output_size, flags);
			}
		}
    }
    else
    {
		LOG(error) << "failed write audio frame into audio point - invalid argument: data = " << data << ", size = " << size LOG_END;
    }

    return result;
}

std::int32_t AudioPoint::Read(void* data, std::size_t size, const audio_format_t& audio_format, std::uint32_t flags)
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

			if (input_size > m_input_resample_buffer.size())
			{
				m_input_resample_buffer.resize(input_size);
			}

			result = Read(m_input_resample_buffer.data(), input_size, flags);

			if (result > 0)
			{
				result = AudioResampler::Resampling(GetInputFormat(), audio_format, m_input_resample_buffer.data(), result, data, size);
			}
		}
	}
	else
	{
		LOG(error) << "failed read audio frame from audio point - invalid argument: data = " << data << ", size = " << size LOG_END;
	}

	return result;
}

} // audio

} // media

} // core
