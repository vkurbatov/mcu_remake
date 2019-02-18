#ifndef DATA_QUEUE_H
#define DATA_QUEUE_H

#include "media/common/i_data_queue.h"
#include <vector>

namespace core
{

namespace media
{

class DataQueue : public IDataQueue
{
	typedef std::vector<std::uint8_t> queue_buffer_t;

	queue_buffer_t  m_buffer;
	std::uint32_t   m_position;
	std::size_t     m_size;

public:

	DataQueue(std::size_t capacity);
	virtual ~DataQueue() override = default;

	// IDataQueue interface
public:
	std::size_t Pop(void* data, std::size_t size) override;
	std::size_t Read(void* data, std::size_t size) const override;
	std::size_t Drop(std::size_t size) override;
	std::size_t Push(const void* data, std::size_t size) override;

	void Reset(std::size_t capacity = 0) override;
	std::size_t Size() const override;
	std::size_t Capacity() const override;

private:

	std::size_t internal_pop(void* data, std::size_t size);
	std::size_t internal_read(void* data, std::size_t size) const;
	std::size_t internal_drop(std::size_t size);
	std::size_t internal_push(const void* data, std::size_t size);
	void internal_reset(std::size_t capacity = 0);

};

} // media

} // core

#endif // DATA_QUEUE_H
