#ifndef AUDIO_COMPOSER_H
#define AUDIO_COMPOSER_H

#include "media/common/data_queue.h"
#include "media/common/media_queue.h"
#include "media/audio/i_audio_composer.h"
#include "media/audio/audio_format.h"
#include <unordered_map>

namespace core
{

namespace media
{

namespace audio
{

class AudioComposer : public IAudioComposer
{

	static const std::uint32_t max_queue_duration = 10;

	using audio_slot_t = std::shared_ptr<IAudioSlot>;
	using audio_slot_map_t = std::unordered_map<audio_slot_id_t, audio_slot_t>;

	audio_format_t				m_audio_format;
	MediaQueue					m_media_queue;
	audio_slot_map_t			m_audio_slots;

	class SlotCollectionWrapper : public IDataCollection
	{
		audio_slot_map_t& m_audio_slots;
	public:
		explicit SlotCollectionWrapper(audio_slot_map_t& audio_slots);
		// IDataCollection interface
	public:
		std::size_t Count() const override;
	} m_slot_collection;

	//SlotCollectionWrapper		m_slot_collection;

public:
	AudioComposer(const audio_format_t& audio_format, std::uint32_t queue_duration_ms = max_queue_duration);
	~AudioComposer() override = default;

	// IDataQueueControl interface
public:
	void Reset() override;
	std::size_t Size() const override;
	std::size_t Capacity() const override;

	// IAudioComposer interface
public:
	IAudioSlot* operator [](audio_slot_id_t audio_slot_id) override;
	const IAudioSlot* operator [](audio_slot_id_t audio_slot_id) const override;

	IAudioSlot* QueryAudioSlot(audio_slot_id_t audio_slot_id) override;
	std::size_t ReleaseAudioSlot(audio_slot_id_t audio_slot_id) override;

	const audio_format_t& GetAudioFormat() const override;
	bool SetAudioFormat(const audio_format_t& audio_format) override;

	// IDataCollection interface
public:
	std::size_t Count() const override;
};


} // audio

} // media

} // core

#endif // AUDIO_COMPOSER_H
