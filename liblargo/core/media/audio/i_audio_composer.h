#ifndef I_AUDIO_COMPOUSER_H
#define I_AUDIO_COMPOUSER_H

#include "media/common/i_data_queue.h"
#include "media/common/i_data_collection.h"
#include "media/audio/i_audio_slot.h"

namespace core
{

namespace media
{

namespace audio
{

#ifndef AUDIO_FORMAT_H
struct audio_format_t;
#endif

class IAudioComposer : public IAudioFormatter, public IDataQueueControl, public IDataCollection
{

public:
	virtual ~IAudioComposer() override{}

	virtual IAudioSlot* operator[](audio_slot_id_t audio_slot_id) = 0;
	virtual const IAudioSlot* operator[](audio_slot_id_t audio_slot_id) const = 0;

	virtual IAudioSlot* QueryAudioSlot(audio_slot_id_t audio_slot_id) = 0;
	virtual std::size_t ReleaseAudioSlot(audio_slot_id_t audio_slot_id) = 0;
};

} // audio

} // media

} // core

#endif // I_AUDIO_COMPOUSER_H
