#ifndef I_PACKETIZER_H
#define I_PACKETIZER_H

#include <cstdint>

namespace largo
{

namespace codec
{

enum class packetizer_direction_t
{
	packetizer,
	depacketizer
};

class IPacketizer
{
public:
	virtual ~IPacketizer() {}
	virtual std::size_t Push(const void* data, std::size_t size) = 0;
	virtual std::size_t Pop(void* data, std::size_t size = 0) = 0;
	virtual void Reset() = 0;
	virtual std::size_t Count() const = 0;
	virtual packetizer_direction_t GetDirection() const = 0;
};

} // codec

} //largo

#endif // I_PACKETIZER_H
