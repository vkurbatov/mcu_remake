#include "media_queue.h"
#include "media/common/media_slot.h"

namespace core
{

namespace media
{

MediaQueue::MediaQueue(std::size_t capacity)
	: m_multipoint_data_queue(capacity)
{

}

IMediaSlot* MediaQueue::operator [](media_slot_id_t media_slot_id)
{
	auto it = m_media_slots.find(media_slot_id);

	return it != m_media_slots.end() ? it->second.get() : nullptr;
}

const IMediaSlot* MediaQueue::operator [](media_slot_id_t media_slot_id) const
{
	auto it = m_media_slots.find(media_slot_id);

	return it != m_media_slots.end() ? it->second.get() : nullptr;
}

IMediaSlot* MediaQueue::AddSlot(media_slot_id_t media_slot_id)
{
	auto result = operator[](media_slot_id);

	if (result == nullptr)
	{
		media_slot_t media_slot(new MediaSlot(media_slot_id, m_multipoint_data_queue), [](IMediaSlot* slot){ delete static_cast<MediaSlot*>(slot); });
		result = media_slot.get();
		m_media_slots.emplace(std::make_pair(media_slot_id, media_slot));
	}

	return result;
}

bool MediaQueue::RemoveSlot(media_slot_id_t media_slot_id)
{
	auto it = m_media_slots.find(media_slot_id);

	bool result = (it != m_media_slots.end());

	if (result == true)
	{
		m_media_slots.erase(it);
	}

	return result;
}

void MediaQueue::Reset()
{
	m_multipoint_data_queue.Reset();
}

std::size_t MediaQueue::Size() const
{
	return m_multipoint_data_queue.Size();
}

std::size_t MediaQueue::Capacity() const
{
	return m_multipoint_data_queue.Capacity();
}

} // media

} // core
