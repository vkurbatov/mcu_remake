#ifndef AUDIO_SLOT_H
#define AUDIO_SLOT_H

#include "media/common/data_queue.h"
#include "media/common/i_media_slot.h"
#include "media/audio/i_audio_slot.h"

namespace core
{

namespace media
{


namespace audio
{

#ifndef AUDIO_FORMAT_H
class audio_format_t;
#endif

class AudioSlot : public IAudioSlot
{
	friend class AudioComposer;

	std::size_t					m_ref_count;

	DataQueue					m_palyback_queue;
	const audio_format_t&		m_audio_format;
	IMediaSlot&					m_media_slot;

	std::vector<std::uint32_t>	m_input_resampler_buffer;
	std::vector<std::uint32_t>	m_output_resampler_buffer;

	AudioSlot(const audio_format_t& audio_format, IMediaSlot& media_slot);
	~AudioSlot() override = default;

public:

	// IAudioPoint interface
public:
	std::int32_t Write(const void* data, std::size_t size, const audio_format_t& audio_format, uint32_t options = 0) override;
	std::int32_t Read(void* data, std::size_t size, const audio_format_t& audio_format, uint32_t options = 0) override;

	// IAudioSlot interface
public:
	audio_slot_id_t GetSlotId() const override;
	bool IsSkip() const override;
};

} // audio

} // media

} // core

#endif // AUDIO_SLOT_H
