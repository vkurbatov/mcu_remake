#ifndef AUDIO_CHANNEL_H
#define AUDIO_CHANNEL_H

#include "media/audio/channels/i_audio_channel.h"
#include "media/audio/channels/audio_channel_params.h"
#include "media/audio/audio_point.h"

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

public:

	AudioChannel();
	virtual ~AudioChannel() override = default;

	// IAudoChannel interface
public:

	const audio_channel_params_t& GetAudioParams() const override final;
	bool SetAudioParams(const audio_channel_params_t& audio_params) override final;

	virtual bool IsRecorder() const override;
	virtual bool IsPlayback() const override;

	// IAudioFormatter interface
public:

	const audio_format_t &GetAudioFormat() const override;
	bool SetAudioFormat(const audio_format_t &audio_format) override;

protected:

	virtual const audio_channel_params_t& internal_get_audio_params() const = 0;
	virtual bool internal_set_audio_params(const audio_channel_params_t& audio_params) = 0;


};

} // channels

} // audio

} // media

} // core


#endif // AUDIO_CHANNEL_H
