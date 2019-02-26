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

AudioPoint::AudioPoint(const audio_format_t& audio_format)
	: m_resampler(audio_format)
{

}

std::int32_t AudioPoint::Write(const audio_format_t& audio_format, const void* data, std::size_t size, std::uint32_t options)
{
	std::int32_t result = -EINVAL;

	if (data != nullptr && size > 0)
	{
		if (audio_format == GetAudioFormat())
		{
			result = Write(data, size, options);
		}
		else
		{
			result = 0;

			auto& resample_buffer = m_resampler(audio_format, data, size);

			if (resample_buffer.size() > 0)
			{
				result = Write(resample_buffer.data(), resample_buffer.size(), options);
			}
		}
	}
	else
	{
		LOG(error) << "Failed write audio frame into audio point - invalid argument: data = " << data << ", size = " << size LOG_END;
	}

	return result;
}

std::int32_t AudioPoint::Read(const audio_format_t& audio_format, void* data, std::size_t size, std::uint32_t options)
{
	std::int32_t result = -EINVAL;

	if (data != nullptr && size > 0)
	{
		if (audio_format == GetAudioFormat())
		{
			result = Read(data, size, options);
		}
		else
		{
			auto input_size = GetAudioFormat().size_from_format(audio_format, size);

			if (input_size > m_input_resampler_buffer.size())
			{
				m_input_resampler_buffer.resize(input_size);
			}

			result = Read(m_input_resampler_buffer.data(), input_size, options);

			if (result > 0)
			{	
				result = m_resampler(audio_format, m_input_resampler_buffer.data(), result, data, size, true);
			}
		}
	}
	else
	{
		LOG(error) << "Failed read audio frame from audio point - invalid argument: data = " << data << ", size = " << size LOG_END;
	}

	return result;
}

const audio_format_t& AudioPoint::GetAudioFormat() const
{
	return m_resampler.GetAudioFormat();
}

bool AudioPoint::SetAudioFormat(const audio_format_t& audio_format)
{
	m_resampler.SetAudioFormat(audio_format);
	return true;
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

		m_volume_controller(GetAudioFormat().sample_format, data, size, m_output_volume_buffer.data());

		data_ptr = m_output_volume_buffer.data();
	}

	return MediaPoint::Write(data_ptr, size, flags);
}

int32_t AudioPoint::Read(void* data, std::size_t size, uint32_t flags)
{

	auto result = MediaPoint::Read(data, size, flags);

	m_volume_controller(GetAudioFormat().sample_format, data, result);

	return result;

}

} // audio

} // media

} // core
