#ifndef I_AUDIO_CHANNEL_H
#define I_AUDIO_CHANNEL_H

//#include <cstdint>
#include <string>


namespace core
{

namespace media
{

namespace audio
{

namespace channels
{

#ifndef AUDIO_CHANNEL_PARAMS_H
struct audio_channel_params_t;
#endif

class IAudoChannel
{
public:

	static const std::uint32_t max_volume = 100u;
	static const std::uint32_t min_volume = 0u;

	virtual ~IAudoChannel() = default;

	virtual std::int32_t Open(const std::string& device_name) = 0;
	virtual std::int32_t Close() = 0;
	virtual bool IsOpen() const = 0;

	virtual const audio_channel_params_t& GetAudioParams() const = 0;
	virtual bool SetAudioParams(const audio_channel_params_t& audio_params) = 0;

	virtual bool IsReader() const = 0;
	virtual bool IsPlayback() const = 0;

	virtual std::uint32_t GetVolume() const = 0;
	virtual void SetVolume(std::uint32_t volume) = 0;

	virtual bool GetMute() const = 0;
	virtual void SetMute(bool mute) = 0;

	virtual const std::string& GetName() const = 0;

};

} // channels

} // audio

} // media

} // core

#endif // I_AUDIO_CHANNEL_H
