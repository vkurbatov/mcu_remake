#ifndef MEDIA_SLOT_H
#define MEDIA_SLOT_H

#include "core/media/common/i_media_slot.h"
#include "core/media/common/i_multipoint_data_queue.h"

namespace core
{

namespace media
{

class MediaSlot : public IMediaSlot
{
	friend class MediaQueue;

	cursor_t				m_read_cursor;
	cursor_t				m_write_cursor;
	media_slot_id_t			m_media_slot_id;

	std::size_t				m_ref_count;

	// Dependencies
private:
	IMultipointDataQueue&	m_multipoint_data_queue;

private:
	explicit MediaSlot(media_slot_id_t media_slot_id, IMultipointDataQueue& multipoint_data_queue);
	virtual ~MediaSlot() override {}

	MediaSlot(const MediaSlot&) = delete;
	MediaSlot(MediaSlot&&) = delete;
	MediaSlot& operator=(const MediaSlot&) = delete;
	MediaSlot& operator=(MediaSlot&&) = delete;

	// IDataQueue interface
public:
	std::size_t Pop(void* data, std::size_t size) override;
	std::size_t Read(void* data, std::size_t size, bool from_tail = false) const override;
	std::size_t Drop(std::size_t size) override;
	std::size_t Push(const void* data, std::size_t size) override;

	void Reset() override;
	std::size_t Size() const override;
	std::size_t Capacity() const override;

	// IMediaSlot interface
public:

	media_slot_id_t GetSlotId() const override;
	std::size_t ReadJitter() const override;
	std::size_t WriteJitter() const override;

private:

	std::size_t internal_pop(void* data, std::size_t size);
	std::size_t internal_read(void* data, std::size_t size, bool from_tail) const;
	std::size_t internal_drop(std::size_t size);
	std::size_t internal_push(const void* data, std::size_t size);
	void internal_reset();

};

} //media

} //core

#endif // MEDIA_SLOT_H
