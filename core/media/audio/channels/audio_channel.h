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

	AudioChannel(const audio_channel_params_t& audio_params = null_audio_params);
	virtual ~AudioChannel() override = default;

	// IAudoChannel interface
public:

	const audio_channel_params_t& GetAudioParams() const override final;
	bool SetAudioParams(const audio_channel_params_t& audio_params) override final;

	virtual bool IsRecorder() const override;
	virtual bool IsPlayback() const override;

	// IAudioFormatter interface
public:

	const audio_format_t &GetInputFormat() const override;
	const audio_format_t &GetOutputFormat() const override;
	void SetInputFormat(const audio_format_t &input_format) override;
	void SetOutputFormat(const audio_format_t &output_format) override;

protected:

	virtual const audio_channel_params_t& internal_get_audio_params() const = 0;
	virtual bool internal_set_audio_params(const audio_channel_params_t& audio_params) = 0;


};

} // channels

} // audio

} // media

} // core


#endif // AUDIO_CHANNEL_H
