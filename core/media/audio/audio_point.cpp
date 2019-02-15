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
	: m_input_resampler(input_format, input_format)
	, m_output_resampler(output_format, output_format)
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

			auto& resample_buffer = m_output_resampler(data, size, audio_format);

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

			if (input_size > m_input_resampler_buffer.size())
			{
				m_input_resampler_buffer.resize(input_size);
			}

			result = Read(m_input_resampler_buffer.data(), input_size, flags);

			if (result > 0)		
			{
				if (audio_format != m_input_resampler.GetOutputFormat())
				{
					m_input_resampler.SetOutputFormat(audio_format);
				}
				result = m_input_resampler(m_input_resampler_buffer.data(), result, data, size);
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
	return m_input_resampler.GetInputFormat();
}

const audio_format_t& AudioPoint::GetOutputFormat() const
{
	return m_output_resampler.GetOutputFormat();
}

void AudioPoint::SetInputFormat(const audio_format_t& input_fromat)
{
	m_input_resampler.SetInputFormat(input_fromat);
}

void AudioPoint::SetOutputFormat(const audio_format_t& output_fromat)
{
	m_output_resampler.SetOutputFormat(output_fromat);
}

uint32_t AudioPoint::GetVolume() const
{
	return m_volume_controller.GetVolume();
}

void AudioPoint::SetVolume(uint32_t volume)
{
	m_volume_controller.SetVolume(volume);
}

bool AudioPoint::IsMute() const
{
	return m_volume_controller.IsMute();
}

void AudioPoint::SetMute(bool mute)
{
	m_volume_controller.SetMute(mute);
}

int32_t AudioPoint::Write(const void* data, std::size_t size, uint32_t flags)
{

	auto data_ptr = data;

	if (GetVolume() != max_volume || IsMute() == true)
	{
		if (m_output_volume_buffer.size() < size)
		{
			m_output_volume_buffer.resize(size);
		}

		m_volume_controller(GetOutputFormat().bit_per_sample, data, size, m_output_volume_buffer.data());

		data_ptr = m_output_volume_buffer.data();
	}

	return MediaPoint::Write(data_ptr, size, flags);
}

int32_t AudioPoint::Read(void* data, std::size_t size, uint32_t flags)
{

	auto result = MediaPoint::Read(data, size, flags);

	m_volume_controller(GetInputFormat().bit_per_sample, data, result);

	return result;

}

} // audio

} // media

} // core
