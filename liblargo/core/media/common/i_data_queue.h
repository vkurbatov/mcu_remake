#ifndef I_DATA_QUEUE_H
#define I_DATA_QUEUE_H

#include <cstdint>

namespace core
{

namespace media
{

class IDataQueueControl
{
public:
    virtual ~IDataQueueControl() {}

	virtual void Reset() = 0;
	virtual std::size_t Size() const = 0;
	virtual std::size_t Capacity() const = 0;
};

class IDataQueueReader
{
public:
    virtual ~IDataQueueReader() {}

	virtual std::size_t Pop(void* data, std::size_t size) = 0;
	virtual std::size_t Read(void* data, std::size_t size, bool from_tail = false) const = 0;
	virtual std::size_t Drop(std::size_t size) = 0;
};

class IDataQueueWriter
{
public:
    virtual ~IDataQueueWriter() {}

	virtual std::size_t Push(const void* data, std::size_t size) = 0;
};

class IDataQueueIO : public IDataQueueReader, public IDataQueueWriter
{
public:
    virtual ~IDataQueueIO() override {}
};

class IDataQueue : public IDataQueueControl, public IDataQueueIO
{

public:

    virtual ~IDataQueue() override {}

};

} // media

} // core

#endif // I_DATA_QUEUE_H
