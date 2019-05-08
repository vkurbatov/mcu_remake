#ifndef AAC_AUDIO_PACKETIZER_H
#define AAC_AUDIO_PACKETIZER_H

#include "codec/audio/audio_packetizer.h"
#include "codec/audio/aac/aac_types.h"

namespace largo
{

namespace codec
{

namespace audio
{

class AacAudioPacketizer : public AudioPacketizer
{
	au_header_options_t			m_au_packetize_options;
	std::uint32_t				m_au_index;
	std::size_t					m_max_payload_size;


public:
	AacAudioPacketizer(bool is_packetizer);
	virtual ~AacAudioPacketizer() {}
	const au_header_options_t& GetPacketizeOptions() const;

	// AudioPacketizer interface
protected:
	std::size_t internal_get_packet_size(const void *data, std::size_t size) const override;
	std::size_t internal_packetize(const void *input_data, std::size_t input_size, void *output_data, std::size_t output_size) override;

private:

	std::size_t get_au_header_size(bool first_frame = true) const;
	std::size_t get_payload_size(const void* au_header, std::size_t size) const;
	bool is_first_frame() const;
	bool is_valid_au_options() const;
};

} // audio

} // codec

} // largo

#endif // AAC_AUDIO_PACKETIZER_H
