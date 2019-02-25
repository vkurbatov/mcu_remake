#ifndef I_AUDIO_DISPATCHER_H
#define I_AUDIO_DISPATCHER_H

//#include <cstdint>

#include "media/audio/i_audio_point.h"
#include "media/audio/channels/i_audio_channel.h"

namespace core
{

namespace media
{

namespace audio
{

namespace channels
{

class IAudioDispatcher : public IAudioPoint, public IAudoChannel
{
public:
	virtual ~IAudioDispatcher() = default;


};

} // channels

} // audio

} // media

} // core

#endif // I_AUDIO_DISPATCHER_H
