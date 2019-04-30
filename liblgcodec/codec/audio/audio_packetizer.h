#ifndef AUDIO_PACKETIZER_H
#define AUDIO_PACKETIZER_H

#include "codec/common/i_packetizer.h"
#include <vector>
#include <queue>

namespace largo
{

namespace codec
{

namespace audio
{



class AudioPacketizer : virtual public IPacketizer
{
	using packet_t = std::vector<std::uint8_t>;
	using packet_queue_t = std::queue<packet_t>;

	packet_queue_t	m_packet_queue;
	bool			m_is_packetizer;

public:
	AudioPacketizer(bool is_packetizer);
	virtual ~AudioPacketizer() {}

	// IPacketizer interface
public:
	virtual std::int32_t Push(const void* data, std::size_t size) override;
	virtual std::int32_t Pop(void* data, std::size_t size) override;
	virtual void Reset() override;
	virtual std::size_t Count() const override;
	packetizer_direction_t GetDirection() const override;

protected:

	virtual std::int32_t internal_get_packet_size(const void* data, std::size_t size) const = 0;
	virtual std::int32_t internal_packetize(const void* input_data, std::size_t input_size, void* output_data, std::size_t output_size) = 0;

};

} // audio

} // codec

} // largo

#endif // AUDIO_PACKETIZER_H
