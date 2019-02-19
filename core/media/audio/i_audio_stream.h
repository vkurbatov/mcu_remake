#ifndef I_AUDIO_STREAM_H
#define I_AUDIO_STREAM_H

#include "media/common/i_media_stream.h"
#include "media/audio/i_audio_point.h"


namespace core
{

namespace media
{

namespace audio
{

class IAudioStream : public IMediaStream, public IAudioPoint
{

public:
    virtual ~IAudioStream() = default;

};

} // audio

} // media

} // core

#endif // I_AUDIO_STREAM_H
