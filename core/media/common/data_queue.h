#ifndef DATA_QUEUE_H
#define DATA_QUEUE_H

#include <cstdint>
#include <vector>

namespace core
{

namespace media
{

class DataQueue
{
    typedef std::vector<std::uint8_t> queue_buffer_t;

    queue_buffer_t  m_buffer;
    std::uint32_t   m_position;
    std::size_t     m_size;

public:

    DataQueue(std::size_t size);

    std::size_t Pop(void* data, std::size_t size);
    std::size_t Get(void* data, std::size_t size);
    std::size_t Drop(std::size_t size);
    std::size_t Push(const void* data, std::size_t size);

    void Reset(std::size_t capacity = 0);
    inline std::size_t Size() const { return m_size; }
    inline std::size_t Capacity() const { return m_buffer.size(); }

private:

	std::size_t internal_pop(void* data, std::size_t size);
	std::size_t internal_get(void* data, std::size_t size);
	std::size_t internal_drop(std::size_t size);
	std::size_t internal_push(const void* data, std::size_t size);
	void internal_reset(std::size_t capacity = 0);

};

} // media

} // core

#endif // DATA_QUEUE_H
