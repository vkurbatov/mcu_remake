#ifndef I_MEDIA_SLOT_H
#define I_MEDIA_SLOT_H

#include "media/common/i_data_queue.h"

namespace core
{

namespace media
{

using media_slot_id_t = std::uint32_t;

class IMediaSlot : public IDataQueue
{

friend class IMediaQueue;

protected:
	virtual ~IMediaSlot() override = default;

public:

	virtual media_slot_id_t GetSlotId() const = 0;

};

}

}

#endif // I_MEDIA_SLOT_H
