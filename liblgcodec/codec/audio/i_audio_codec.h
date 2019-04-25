#ifndef I_AUDIO_CODEC_H
#define I_AUDIO_CODEC_H

#include "codec/common/i_codec.h"
#include "codec/audio/audio_codec_types.h"

namespace largo
{

namespace codec
{

namespace audio
{

class IAudioCodec : virtual public ICodec
{
public:
	virtual ~IAudioCodec() {}
	virtual audio_codec_id_t GetCodecId() const = 0;
};

} // audio

} // codec

} // largo

#endif // I_AUDIO_CODEC_H
