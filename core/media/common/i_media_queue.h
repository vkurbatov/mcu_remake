#ifndef I_MEDIA_QUEUE_H
#define I_MEDIA_QUEUE_H

#include "media/common/i_data_queue.h"

namespace core
{

namespace media
{

using queue_slot_id_t = std::uint32_t;

class IMediaQueue
{

public:
	virtual ~IMediaQueue() = default;

	virtual IDataQueue* GetQueueSlot(queue_slot_id_t id) = 0;
	// virtual

};

} //media

} //core

#endif // I_MEDIA_QUEUE_H
