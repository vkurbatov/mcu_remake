#include "media_queue.h"
#include "core/media/common/media_slot.h"

namespace core
{

namespace media
{

MediaQueue::MediaQueue(std::size_t capacity, bool is_strong)
	: m_multipoint_data_queue(capacity, is_strong)
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

IMediaSlot* MediaQueue::QuerySlot(media_slot_id_t media_slot_id)
{
	auto result = operator[](media_slot_id);

	if (result == nullptr)
	{
		media_slot_t media_slot(new MediaSlot(media_slot_id, m_multipoint_data_queue), [](IMediaSlot * slot)
		{
			delete static_cast<MediaSlot*>(slot);
		});

		result = media_slot.get();

		if (result != nullptr)
		{
			m_media_slots.emplace(std::make_pair(media_slot_id, std::move(media_slot)));
		}
	}
	else
	{
		static_cast<MediaSlot*>(result)->m_ref_count++;
	}

	return result;
}

std::size_t MediaQueue::ReleaseSlot(media_slot_id_t media_slot_id)
{
	auto it = m_media_slots.find(media_slot_id);

	std::size_t result = 0;

	if (it != m_media_slots.end())
	{
		auto& slot = static_cast<MediaSlot&>(*it->second);

		slot.m_ref_count -= static_cast<std::size_t>(slot.m_ref_count > 0);

		if ((result = slot.m_read_cursor) == 0)
		{
			m_media_slots.erase(it);
		}

	}

	return result;
}

std::size_t MediaQueue::Count() const
{
	return m_media_slots.size();
}

void MediaQueue::Reset()
{
	m_multipoint_data_queue.Reset();

	for (auto &s : m_media_slots)
	{
		s.second->Reset();
	}
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
