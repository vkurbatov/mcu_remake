#ifndef DATA_QUEUE_H
#define DATA_QUEUE_H

#include <cstdint>
#include <vector>

class DataQueue
{
    typedef std::vector<std::uint8_t> aec_buffer_t;

    aec_buffer_t    m_buffer;
    std::uint32_t   m_position;
    std::size_t     m_size;

public:

    DataQueue(std::size_t size);

    std::size_t Pop(void* data, std::size_t size);
    std::size_t Push(const void* data, std::size_t size);

    void Reset(std::size_t capacity = 0);
    inline std::size_t Size() const { return m_size; }
    inline std::size_t Capacity() const { return m_buffer.size(); }

};

#endif // DATA_QUEUE_H
