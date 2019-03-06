#ifndef VOLUME_CONTROLLER_H
#define VOLUME_CONTROLLER_H

#include "i_volume_controller.h"
#include "audio_format.h"

namespace core
{

namespace media
{

namespace audio
{

class VolumeController : public IVolumeController
{
public:

	std::uint32_t	m_volume;
	bool			m_mute;


	VolumeController(std::uint32_t volume = max_volume);
	virtual ~VolumeController() override = default;

	static std::uint32_t VolumeChange(audio_format_t::sample_format_t sample_format, std::uint32_t volume, const void* input_data, std::size_t input_size, void* output_data, std::size_t output_size = 0);
	static std::uint32_t VolumeChange(audio_format_t::sample_format_t sample_format, std::uint32_t volume, void* data, std::size_t size);

	std::uint32_t operator()(audio_format_t::sample_format_t sample_format, const void* input_data, std::size_t input_size, void* output_data, std::size_t output_size = 0);
	std::uint32_t operator()(audio_format_t::sample_format_t sample_format, void* data, std::size_t size);

	// IVolumeController interface
public:
	virtual std::uint32_t GetVolume() const override;
	virtual void SetVolume(std::uint32_t volume) override;

	virtual bool IsMute() const override;
	virtual void SetMute(bool mute) override;

};

} // audio

} // media

} // core

#endif // VOLUME_CONTROLLER_H
