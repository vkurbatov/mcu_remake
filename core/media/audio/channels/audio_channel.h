#ifndef AUDIO_CHANNEL_H
#define AUDIO_CHANNEL_H

#include "i_audio_channel.h"
#include "media/audio/audio_point.h"
#include "audio_channel_params.h"

namespace core
{

namespace media
{

namespace audio
{

namespace channels
{

class AudioChannel : public IAudoChannel, public AudioPoint
{

	audio_channel_params_t	m_audio_params;
	bool					m_mute;
	std::uint32_t			m_volume;

public:

	AudioChannel(const audio_channel_params_t& param = null_audio_params);

	// IAudoChannel interface
public:

	virtual int32_t Open(const std::string& device_name) override;
	virtual int32_t Close() override;
	virtual bool IsOpen() const override;

	virtual const audio_channel_params_t& GetAudioParams() const override;
	virtual bool SetAudioParams(const audio_channel_params_t& audio_params) override;

	virtual bool IsReader() const override;
	virtual bool IsPlayback() const override;

	virtual uint32_t GetVolume() const override;
	virtual void SetVolume(uint32_t volume) override;

	virtual bool GetMute() const override;
	virtual void SetMute(bool mute) override;

	virtual const std::string& GetName() const override;

};

} // channels

} // audio

} // media

} // core


#endif // AUDIO_CHANNEL_H
