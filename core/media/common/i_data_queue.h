#ifndef I_DATA_QUEUE_H
#define I_DATA_QUEUE_H

#include <cstdint>

namespace core
{

namespace media
{

class IDataQueue
{

public:

	virtual ~IDataQueue() = default;

	virtual std::size_t Pop(void* data, std::size_t size) = 0;
	virtual std::size_t Read(void* data, std::size_t size) const = 0;
	virtual std::size_t Drop(std::size_t size) = 0;
	virtual std::size_t Push(const void* data, std::size_t size) = 0;

	virtual void Reset(std::size_t capacity = 0) = 0;
	virtual std::size_t Size() const = 0;
	virtual std::size_t Capacity() const = 0;

};

} // media

} // core

#endif // I_DATA_QUEUE_H
