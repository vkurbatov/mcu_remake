#ifndef I_AUDIO_CHANNEL_H
#define I_AUDIO_CHANNEL_H

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

	virtual ~IAudoChannel() {}

	virtual bool Open(const std::string& device_name) = 0;
	virtual bool Close() = 0;
	virtual bool IsOpen() const = 0;

	virtual const audio_channel_params_t& GetAudioParams() const = 0;
	virtual bool SetAudioParams(const audio_channel_params_t& audio_params) = 0;

	virtual bool IsRecorder() const = 0;
	virtual bool IsPlayback() const = 0;

	virtual const std::string& GetName() const = 0;

};

} // channels

} // audio

} // media

} // core

#endif // I_AUDIO_CHANNEL_H
