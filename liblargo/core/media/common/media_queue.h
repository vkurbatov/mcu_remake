#ifndef MEDIA_QUEUE_H
#define MEDIA_QUEUE_H

#include "media/common/i_media_queue.h"
#include "media/common/multipoint_data_queue.h"
#include <unordered_map>
#include <memory>


namespace core
{

namespace media
{

class MediaQueue : public IMediaQueue
{

	using media_slot_t = std::shared_ptr<IMediaSlot>;
	using slot_map_t = std::unordered_map<media_slot_id_t, media_slot_t>;

	slot_map_t					m_media_slots;
	MultipointDataQueue			m_multipoint_data_queue;

public:
	MediaQueue(std::size_t capacity, bool is_strong = false);
	~MediaQueue() override{}

	// IMediaQueue interface
public:
	IMediaSlot* operator [](media_slot_id_t media_slot_id) override;
	const IMediaSlot* operator [](media_slot_id_t media_slot_id) const override;
	IMediaSlot* QuerySlot(media_slot_id_t media_slot_id) override;
	std::size_t ReleaseSlot(media_slot_id_t media_slot_id) override;

	std::size_t Count() const override;

	// IDataQueueControl interface
public:
	void Reset() override;
	std::size_t Size() const override;
	std::size_t Capacity() const override;
};

} // media

} // core

#endif // MEDIA_QUEUE_H
