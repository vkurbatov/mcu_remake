#include "multipoint_data_queue.h"
#include <cstring>

namespace core
{

namespace media
{

MultipointDataQueue::MultipointDataQueue(std::size_t capacity, bool is_strong)
	: m_buffer(capacity + 1)
	, m_cursor(0)
	, m_size(0)
	, m_is_strong(is_strong)
{

}

std::size_t MultipointDataQueue::Read(cursor_t cursor, void* data, std::size_t size) const
{
	return internal_read(cursor, data, size);
}

std::size_t MultipointDataQueue::Write(cursor_t cursor, const void* data, std::size_t size)
{
	return internal_write(cursor, data, size);
}

void MultipointDataQueue::Reset()
{
	return internal_reset();
}

std::size_t MultipointDataQueue::Size() const
{
	return m_size;
}

std::size_t MultipointDataQueue::GetDataSize(cursor_t cursor, bool is_before) const
{
	return get_data_size(cursor, is_before);
}

std::size_t MultipointDataQueue::Capacity() const
{
	return m_buffer.size() - 1;
}

cursor_t MultipointDataQueue::GetWriteCursor() const
{
	return m_cursor;
}

cursor_t MultipointDataQueue::GetReadCursor() const
{
	return m_cursor - m_size;
}

void MultipointDataQueue::Resize(std::size_t capacity)
{
	internal_resize(capacity);
}

std::size_t MultipointDataQueue::internal_read(cursor_t cursor, void* data, std::size_t size) const
{
	std::size_t result = 0;

	if (is_valid_cursor(cursor))
	{

		auto real_size = get_data_size(cursor);

		if ((size <= real_size) || (m_is_strong == false))
		{
			auto data_ptr = static_cast<std::uint8_t*>(data);
			auto buffer_size = m_buffer.size();

			size = std::min(size, real_size);

			auto position = get_position(cursor);
			auto tail = position + size;

			result = size;

			if (tail >= buffer_size)
			{
				auto part_size = buffer_size - position;

				std::memcpy(data_ptr, m_buffer.data() + position, part_size);

				data_ptr += part_size;
				size -= part_size;
				position = 0;

			}

			std::memcpy(data_ptr, m_buffer.data() + position, size);

		}

	}

	return result;
}

std::size_t MultipointDataQueue::internal_write(cursor_t cursor, const void* data, std::size_t size)
{
	std::uint32_t result = 0;

	if (is_valid_cursor(cursor))
	{
		auto data_ptr = static_cast<const std::uint8_t*>(data);
		auto buffer_size = m_buffer.size();
		auto last_cursor = cursor + size;

		auto real_position = get_position(cursor);

		if (size >= buffer_size)
		{
			data_ptr += (size - buffer_size + 1);
			real_position += (size - buffer_size + 1);
			size = buffer_size - 1;
		}

		if (size > 0)
		{

			auto tail = real_position + size;

			if (tail > buffer_size)
			{
				auto part_size = buffer_size - real_position;

				std::memcpy(m_buffer.data() + real_position, data_ptr, part_size);
				std::memcpy(m_buffer.data(), data_ptr + part_size, (size - part_size));
			}
			else
			{
				std::memcpy(m_buffer.data() + real_position, data_ptr, size);
			}

			if (m_cursor < last_cursor)
			{
				m_size += last_cursor - m_cursor;
				m_cursor = last_cursor;
			}

			// cut old data
			if (m_size >= buffer_size)
			{
				m_size = buffer_size - 1;
			}

			result = size;
		}
	}

	return result;
}

void MultipointDataQueue::internal_reset()
{
	m_cursor = 0;
	m_size = 0;
}

void MultipointDataQueue::internal_resize(std::size_t capacity)
{
	if (capacity != m_buffer.size())
	{
		m_buffer.resize(capacity + 1);
		internal_reset();
	}
}


bool MultipointDataQueue::is_valid_cursor(cursor_t cursor) const
{
	return cursor <= m_cursor && (m_cursor - cursor) <= m_size;
}

std::size_t MultipointDataQueue::get_data_size(cursor_t cursor, bool is_before) const
{
	return is_valid_cursor(cursor)
	       ? (is_before
	          ? (m_size - (m_cursor - cursor))
	          : (m_cursor - cursor)
	         )
	       : 0;
}

std::uint32_t MultipointDataQueue::get_position(cursor_t cursor) const
{
	return cursor % m_buffer.size();
}

} // media

} // core
