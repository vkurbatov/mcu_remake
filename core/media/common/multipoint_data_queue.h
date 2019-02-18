#ifndef MULTIPOINT_DATA_QUEUE_H
#define MULTIPOINT_DATA_QUEUE_H

#include "media/common/i_multipoint_data_queue.h"
#include "media/common/media_types.h"

namespace core
{

namespace media
{

class MultipointDataQueue : public IMultipointDataQueue
{

	media_buffer_t	m_buffer;
	cursor_t		m_cursor;
	std::size_t		m_size;

public:

	MultipointDataQueue(std::size_t capacity);
	virtual ~MultipointDataQueue() override = default;

	// IMultipointDataQueue interface
public:
	virtual std::size_t Read(cursor_t cursor, void* data, std::size_t size) const override;
	virtual std::size_t Write(cursor_t cursor, const void* data, std::size_t size) override;

	void Reset(std::size_t capacity) override;
	std::size_t Size() const override;
	std::size_t GetDataSize(cursor_t cursor, bool is_before = false) const override;
	std::size_t Capacity() const override;

	cursor_t GetWriteCursor() const override;
	cursor_t GetReadCursor() const override;

private:

	std::size_t internal_read(cursor_t cursor, void* data, std::size_t size) const;
	std::size_t internal_write(cursor_t cursor, const void* data, std::size_t size);

	void internal_reset(std::size_t capacity);

	bool is_valid_cursor(cursor_t cursor) const;
	std::size_t get_data_size(cursor_t cursor, bool is_before = false) const;
	std::uint32_t get_position(cursor_t cursor) const;


};

} // media

} // core


#endif // MULTIPOINT_DATA_QUEUE_H
