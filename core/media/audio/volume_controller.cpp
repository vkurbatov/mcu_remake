#include "volume_controller.h"
#include <cstring>
#include <algorithm>

namespace core
{

namespace media
{

namespace audio
{

namespace volume_utils
{

template<typename Tval>
void change_volume(const void *sound_data, std::size_t size, void *output_data, std::uint32_t volume)
{
	for (int i = 0; i < size / sizeof(Tval); i++)
	{

		auto& input_sample = *(static_cast<const Tval*>(sound_data) + i);
		auto& output_sample = *(static_cast<Tval*>(output_data) + i);

		output_sample = static_cast<Tval>((static_cast<double>(input_sample) * static_cast<double>(volume)) / 100.0f);

	}
}

bool change_volume(std::uint32_t bit_per_sample, const void *sound_data, std::size_t size, void* output_data, std::uint32_t volume)
{

	switch(bit_per_sample)
	{
		case 8:
			change_volume<std::int8_t>(sound_data, size, output_data, volume);
			break;
		case 16:
			change_volume<std::int16_t>(sound_data, size, output_data, volume);
			break;
		case 32:
			change_volume<std::int32_t>(sound_data, size, output_data, volume);
			break;
		default:
			return false;
	}

	return true;
}

} // volume_utils

VolumeController::VolumeController(std::uint32_t volume)
	: m_volume(volume)
	, m_mute(false)
{

}

std::uint32_t VolumeController::VolumeChange(std::uint32_t bit_per_sample, uint32_t volume, const void* input_data, std::size_t input_size, void* output_data, std::size_t output_size)
{

	std::size_t result = std::min(input_size, output_size == 0 ? input_size : output_size);

	volume = std::min(max_volume, std::max(min_volume, volume));

	switch(volume)
	{
		case max_volume:

			if (input_data != output_data)
			{
				std::memcpy(output_data, input_data, result);
			}

			break;
		case min_volume:

			std::memset(output_data, 0, result);

			break;
		default:

			if (volume_utils::change_volume(bit_per_sample, input_data, result, output_data, volume) == false)
			{
				result = 0;
			}
	}

	return result;
}

uint32_t VolumeController::VolumeChange(uint32_t bit_per_sample, uint32_t volume, void* data, std::size_t size)
{
	return VolumeChange(bit_per_sample, volume, data, size, data, size);
}

std::uint32_t VolumeController::operator()(std::uint32_t bit_per_sample, const void* input_data, std::size_t input_size, void* output_data, std::size_t output_size)
{
	return VolumeChange(bit_per_sample, m_mute ? min_volume : m_volume, input_data, input_size, output_data, output_size);
}

uint32_t VolumeController::operator()(uint32_t bit_per_sample, void* data, std::size_t size)
{
	return VolumeChange(bit_per_sample, m_mute ? min_volume : m_volume, data, size);
}

uint32_t VolumeController::GetVolume() const
{
	return m_volume;
}

void VolumeController::SetVolume(uint32_t volume)
{
	m_volume = volume;
}

bool VolumeController::IsMute() const
{
	return m_mute;
}

void VolumeController::SetMute(bool mute)
{
	m_mute = mute;
}


} // audio

} // media

} // core
