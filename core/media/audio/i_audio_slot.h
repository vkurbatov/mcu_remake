#ifndef I_AUDIO_SLOT_H
#define I_AUDIO_SLOT_H

#include "media/audio/i_audio_point.h"
#include "media/audio/i_audio_formatter.h"
#include <cstdint>

namespace core
{

namespace media
{

namespace audio
{

using audio_slot_id_t = std::uint32_t;

class IAudioSlot : public IAudioPoint
{

protected:
	virtual ~IAudioSlot() override = default;

public:

	virtual audio_slot_id_t GetSlotId() const = 0;
	virtual bool IsSkip() const = 0;
};

} // audio

} // media

} // core


#endif // I_AUDIO_SLOT_H
