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

	AudioChannel(const audio_channel_params_t& param = null_audio_params);

	// IAudoChannel interface
public:

	virtual bool IsRecorder() const override;
	virtual bool IsPlayback() const override;


};

} // channels

} // audio

} // media

} // core


#endif // AUDIO_CHANNEL_H
