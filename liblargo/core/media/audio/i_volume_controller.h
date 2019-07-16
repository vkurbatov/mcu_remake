#ifndef I_VOLUME_CONTROLLER_H
#define I_VOLUME_CONTROLLER_H

#include <cstdint>

namespace core
{

namespace media
{

namespace audio
{

static const std::uint32_t max_volume = 100u;
static const std::uint32_t min_volume = 0u;

class IVolumeController
{
public:

	virtual ~IVolumeController() {}

	virtual std::uint32_t GetVolume() const = 0;
	virtual void SetVolume(std::uint32_t volume) = 0;

	virtual bool IsMute() const = 0;
	virtual void SetMute(bool mute) = 0;
};

} // audio

} // media

} // core

#endif // I_VOLUME_CONTROLLER_H
