#ifndef I_MEDIA_QUEUE_H
#define I_MEDIA_QUEUE_H

#include "core/media/common/i_media_slot.h"

namespace core
{

namespace media
{

class IMediaQueue : public IDataQueueControl
{

public:

	virtual ~IMediaQueue() override{}

	virtual IMediaSlot* operator[](media_slot_id_t media_slot_id) = 0;
	virtual const IMediaSlot* operator[](media_slot_id_t media_slot_id) const = 0;

	virtual IMediaSlot* QuerySlot(media_slot_id_t media_slot_id) = 0;
	virtual std::size_t ReleaseSlot(media_slot_id_t media_slot_id) = 0;

	virtual std::size_t Count() const = 0;

};

} //media

} //core

#endif // I_MEDIA_QUEUE_H
