#ifndef I_AUDIO_COMPOUSER_H
#define I_AUDIO_COMPOUSER_H

#include "media/common/i_data_queue.h"
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

class IAudioComposer : public IDataQueueControl
{

public:
	virtual ~IAudioComposer() override = default;

	virtual IAudioSlot* operator[](audio_slot_id_t audio_slot_id) = 0;
	virtual const IAudioSlot* operator[](audio_slot_id_t audio_slot_id) const = 0;

	virtual IAudioSlot* AddAudioSlot(audio_slot_id_t audio_slot_id) = 0;
	virtual std::size_t RemoveAudioSlot(audio_slot_id_t audio_slot_id) = 0;

	virtual const audio_format_t& GetAudioFormat() const = 0;
	virtual bool SetAudioFormat(const audio_format_t& audio_format) = 0;

};

} // audio

} // media

} // core

#endif // I_AUDIO_COMPOUSER_H
