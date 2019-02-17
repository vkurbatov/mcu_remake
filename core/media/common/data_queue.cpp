#include "data_queue.h"
#include <cstring>

namespace core
{

namespace media
{

DataQueue::DataQueue(std::size_t size)
	: m_buffer(size + 1)
	, m_position(0)
	, m_size(0)
{

}

size_t DataQueue::Pop(void* data, std::size_t size)
{
	return internal_pop(data, size);
}

std::size_t DataQueue::Get(void *data, std::size_t size)
{
	return internal_get(data, size);
}

std::size_t DataQueue::Drop(std::size_t size)
{
	return internal_drop(size);
}

size_t DataQueue::Push(const void* data, std::size_t size)
{

	return internal_push(data, size);
}

void DataQueue::Reset(std::size_t capacity)
{
	internal_reset(capacity);
}

std::size_t DataQueue::internal_pop(void *data, std::size_t size)
{
	return internal_drop(internal_get(data, size));
}

std::size_t DataQueue::internal_get(void *data, std::size_t size)
{
	std::size_t result = 0;

	auto data_ptr = static_cast<std::uint8_t*>(data);
	auto buffer_size = m_buffer.size();

	size = std::min(size, m_size);

	auto position = (buffer_size + m_position - m_size) % buffer_size;
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

	return result;
}

std::size_t DataQueue::internal_drop(std::size_t size)
{
	auto result = std::min(m_size, size);

	m_size -= result;

	return result;
}

std::size_t DataQueue::internal_push(const void *data, std::size_t size)
{
	std::uint32_t result = 0;

	auto data_ptr = static_cast<const std::uint8_t*>(data);

	auto buffer_size = m_buffer.size();

	if ( size >= buffer_size)
	{
		data_ptr += (size - buffer_size);
		size = buffer_size - 1;
	}

	if (size <= m_buffer.size())
	{

		auto tail = m_position + size;

		if (tail > buffer_size)
		{
			auto part_size = buffer_size - m_position;

			std::memcpy(m_buffer.data() + m_position, data_ptr, part_size);
			std::memcpy(m_buffer.data(), data_ptr + part_size, (size - part_size));
		}
		else
		{
			std::memcpy(m_buffer.data() + m_position, data_ptr, size);
		}

		m_position = (m_position + size) % buffer_size;

		m_size += size;

		// cut old data
		if (m_size >= buffer_size )
		{
			m_size = buffer_size - 1;
		}
	}

	return result;
}

void DataQueue::internal_reset(std::size_t capacity)
{
	m_position = 0;
	m_size = 0;

	if (capacity != 0)
	{
		m_buffer.resize(capacity + 1);
	}
}

} // media

} // core
