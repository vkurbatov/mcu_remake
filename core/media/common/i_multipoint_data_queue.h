#ifndef I_MULTIPOINT_DATA_QUEUE_H
#define I_MULTIPOINT_DATA_QUEUE_H

#include <cstdint>

namespace core
{

namespace media
{

using cursor_t = std::uint32_t;

class IMultipointDataQueue
{

public:

	virtual ~IMultipointDataQueue() = default;

	virtual std::size_t Read(cursor_t cursor, void* data, std::size_t size) const = 0;
	virtual std::size_t Write(cursor_t cursor, const void* data, std::size_t size) = 0;

	virtual void Reset() = 0;
	virtual std::size_t Size() const = 0;
	virtual std::size_t Capacity() const = 0;
	virtual std::size_t GetDataSize(cursor_t cursor, bool is_before = false) const = 0;

	virtual cursor_t GetWriteCursor() const = 0;
	virtual cursor_t GetReadCursor() const = 0;

};

} // media

} // core

#endif // I_MULTIPOINT_DATA_QUEUE_H
