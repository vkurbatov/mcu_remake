#ifndef I_MEDIA_SLOT_H
#define I_MEDIA_SLOT_H

#include "core/media/common/i_data_queue.h"

namespace core
{

namespace media
{

using media_slot_id_t = std::uint32_t;

class IMediaSlot : public IDataQueue
{

protected:
	virtual ~IMediaSlot() override{}

public:

	virtual media_slot_id_t GetSlotId() const = 0;
	virtual std::size_t ReadJitter() const = 0;
	virtual std::size_t WriteJitter() const = 0;

};

}

}

#endif // I_MEDIA_SLOT_H
