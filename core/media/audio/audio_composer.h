#ifndef AUDIO_COMPOSER_H
#define AUDIO_COMPOSER_H

#include "media/common/data_queue.h"
#include "media/common/i_media_queue.h"
#include "media/common/i_sync_point.h"
#include "media/audio/i_audio_composer.h"
#include "media/audio/audio_format.h"

#include <unordered_map>
#include <memory>
#include <mutex>

namespace core
{

namespace media
{

namespace audio
{

class AudioComposer : public IAudioComposer, private ISyncPoint
{

	static const std::uint32_t max_queue_duration = 10;
	static const std::uint32_t default_jitter_ms = 40;

	using mutex_t = std::mutex;
	using audio_slot_t = std::shared_ptr<IAudioSlot>;
	using audio_slot_map_t = std::unordered_map<audio_slot_id_t, audio_slot_t>;

	audio_format_t				m_audio_format;
	audio_slot_map_t			m_audio_slots;
	std::uint32_t				m_min_jitter_ms;

	IMediaQueue&				m_media_queue;
	mutable mutex_t				m_mutex;

	class SlotCollectionWrapper : public IDataCollection
	{
		audio_slot_map_t& m_audio_slots;
	public:
		explicit SlotCollectionWrapper(audio_slot_map_t& audio_slots);
		// IDataCollection interface
	public:
		std::size_t Count() const override;
	} m_slot_collection;

public:
	AudioComposer(const audio_format_t& audio_format, IMediaQueue& media_queue, std::uint32_t min_jitter_ms = default_jitter_ms);
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

	//IAudioFormatter interface
public:
	const audio_format_t& GetAudioFormat() const override;
	bool SetAudioFormat(const audio_format_t& audio_format) override;

	// IDataCollection interface
public:
	std::size_t Count() const override;

	// ISyncPoint interface
private:
	void Lock() const override;
	void Unlock() const override;
};


} // audio

} // media

} // core

#endif // AUDIO_COMPOSER_H