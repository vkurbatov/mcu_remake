#include "media_slot.h"
#include <algorithm>

namespace core
{

namespace media
{

MediaSlot::MediaSlot(media_slot_id_t media_slot_id,
					 IMultipointDataQueue& multipoint_data_queue)
	: m_media_slot_id(media_slot_id)
	, m_multipoint_data_queue(multipoint_data_queue)
	, m_read_cursor(multipoint_data_queue.GetWriteCursor())
	, m_write_cursor(multipoint_data_queue.GetWriteCursor())
{

}

std::size_t MediaSlot::Pop(void* data, std::size_t size)
{
	return internal_pop(data, size);
}

std::size_t MediaSlot::Read(void* data, std::size_t size, bool from_tail) const
{
	return internal_read(data, size, from_tail);
}

std::size_t MediaSlot::Drop(std::size_t size)
{
	return internal_drop(size);
}

std::size_t MediaSlot::Push(const void* data, std::size_t size)
{
	return internal_push(data, size);
}

void MediaSlot::Reset()
{
	internal_reset();
}

std::size_t MediaSlot::Size() const
{
	return (m_write_cursor + Capacity() - m_read_cursor) % Capacity();
}

std::size_t MediaSlot::Capacity() const
{
	return m_multipoint_data_queue.Capacity();
}

media_slot_id_t MediaSlot::GetSlotId() const
{
	return m_media_slot_id;
}

std::size_t MediaSlot::internal_pop(void* data, std::size_t size)
{

	size = std::min(size, Size());

	auto result = internal_read(data, size, false);

	internal_drop(size);

	return result;
}

std::size_t MediaSlot::internal_read(void* data, std::size_t size, bool from_tail) const
{
	auto cursor = m_write_cursor;

	if (from_tail == false)
	{
		cursor = m_read_cursor;
		size = std::min(size, Size());
	}

	return m_multipoint_data_queue.Read(cursor, data, size);
}

std::size_t MediaSlot::internal_drop(std::size_t size)
{

	m_read_cursor += size;

	if (m_read_cursor > m_write_cursor)
	{
		m_read_cursor = m_write_cursor;
	}

	return size;
}

std::size_t MediaSlot::internal_push(const void* data, std::size_t size)
{
	auto result = m_multipoint_data_queue.Write(m_write_cursor, data, size);

	m_write_cursor += result;

	return result;
}

void MediaSlot::internal_reset()
{
	m_read_cursor = m_write_cursor = m_multipoint_data_queue.GetWriteCursor();
}

} //media

} //core
