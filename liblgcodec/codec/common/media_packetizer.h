#ifndef MEDIA_PACKETIZER_H
#define MEDIA_PACKETIZER_H

#include "codec/common/i_packetizer.h"
#include <vector>
#include <queue>

namespace largo
{

namespace codec
{

namespace audio
{


class MediaPacketizer : virtual public IPacketizer
{

public:
	using media_packet_t = std::vector<std::uint8_t>;
	using media_packet_queue_t = std::queue<media_packet_t>;

protected:
	bool					m_is_packetizer;

public:
	MediaPacketizer(bool is_packetizer);
	virtual ~MediaPacketizer() {}

	// IPacketizer interface
public:
	virtual std::size_t Push(const void* data, std::size_t size) override;
	virtual std::size_t Pop(void* data, std::size_t size) override;
	virtual void Reset() override;
	virtual std::size_t Count() const override;
	packetizer_direction_t GetDirection() const override;

protected:

	virtual std::size_t internal_push(const void* data, std::size_t size) = 0;
	virtual std::size_t internal_pop(void* data, std::size_t size) = 0;
	virtual void internal_reset() = 0;
	virtual std::size_t internal_count() const = 0;
};

} // audio

} // codec

} // largo

#endif // AUDIO_PACKETIZER_H
