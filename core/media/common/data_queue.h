#ifndef DATA_QUEUE_H
#define DATA_QUEUE_H

#include <cstdint>
#include <vector>

namespace core
{

namespace media
{

namespace common
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

    std::size_t internalPop(void* data, std::size_t size);
    std::size_t internalGet(void* data, std::size_t size);
    std::size_t internalDrop(std::size_t size);
    std::size_t internalPush(const void* data, std::size_t size);
    void internalReset(std::size_t capacity = 0);

};

} // common

} // media

} // core

#endif // DATA_QUEUE_H
