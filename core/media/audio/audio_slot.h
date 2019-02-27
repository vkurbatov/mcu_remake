#ifndef AUDIO_SLOT_H
#define AUDIO_SLOT_H

#include "media/common/data_queue.h"
#include "media/common/i_data_collection.h"
#include "media/common/i_media_slot.h"
#include "media/common/i_sync_point.h"
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

	std::vector<std::uint8_t>	m_mix_buffer;
	std::vector<std::uint8_t>	m_demix_buffer;

	const audio_format_t&		m_audio_format;
	IMediaSlot&					m_media_slot;
	const IDataCollection&		m_slots_collection;
	ISyncPoint&					m_sync_point;

	std::vector<std::uint32_t>	m_input_resampler_buffer;
	std::vector<std::uint32_t>	m_output_resampler_buffer;

	AudioSlot(const audio_format_t& audio_format, IMediaSlot& media_slot, const IDataCollection& slot_collection, ISyncPoint& sync_point);
	~AudioSlot() override = default;

public:

	// IAudioPoint interface
public:
	std::int32_t Write(const audio_format_t& audio_format, const void* data, std::size_t size, std::uint32_t options = 0) override;
	std::int32_t Read(const audio_format_t& audio_format, void* data, std::size_t size, std::uint32_t options = 0) override;

	// IAudioSlot interface
public:
	audio_slot_id_t GetSlotId() const override;
	bool IsSkip() const override;

private:
	std::int32_t internal_write(const void* data, std::size_t size, const audio_format_t& audio_format, std::uint32_t options);
	std::int32_t internal_read(void* data, std::size_t size, const audio_format_t& audio_format, uint32_t options);

};

} // audio

} // media

} // core

#endif // AUDIO_SLOT_H
