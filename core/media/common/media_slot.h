#ifndef MEDIA_SLOT_H
#define MEDIA_SLOT_H

#include "media/common/i_media_slot.h"
#include "media/common/i_multipoint_data_queue.h"

namespace core
{

namespace media
{

#ifndef MEDIA_QUEUE_H
class MediaQueue;
#endif

class MediaSlot : public IMediaSlot
{
	friend class MediaQueue;

	cursor_t				m_cursor;
	media_slot_id_t			m_media_slot_id;
	IMultipointDataQueue&	m_multipoint_data_queue;

private:
	MediaSlot(media_slot_id_t media_slot_id, IMultipointDataQueue& multipoint_data_queue, cursor_t cursor);
	virtual ~MediaSlot() override = default;

public:
	// IMediaSlot interface
public:
	media_slot_id_t GetSlotId() const override;
};

} //media

} //core

#endif // MEDIA_SLOT_H
