#ifndef I_MEDIA_SLOT_H
#define I_MEDIA_SLOT_H

#include <cstdint>

namespace core
{

namespace media
{

using media_slot_id_t = std::uint32_t;

class IMediaSlot
{

protected:
	virtual ~IMediaSlot() = default;
public:

	virtual std::int32_t Pop(void* data, std::size_t size) = 0;
	virtual std::int32_t Push(const void* data, std::size_t size) = 0;

	virtual std::int32_t Read(void* data, std::size_t size) = 0;

	virtual media_slot_id_t GetSlotId() const = 0;


};

}

}

#endif // I_MEDIA_SLOT_H
