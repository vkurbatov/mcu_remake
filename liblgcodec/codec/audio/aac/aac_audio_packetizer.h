#ifndef AAC_AUDIO_PACKETIZER_H
#define AAC_AUDIO_PACKETIZER_H

#include "codec/common/media_packetizer.h"
#include "codec/audio/aac/aac_types.h"
#include "codec/audio/aac/au_packetizer.h"


namespace largo
{

namespace codec
{

namespace audio
{

class AacAudioPacketizer : public MediaPacketizer
{
	AuPacketizer	m_au_packetizer;
public:
	AacAudioPacketizer(bool is_packetizer, const aac_header_rules_t& aac_header_rules = default_au_header_rules);
	virtual ~AacAudioPacketizer() {}

	const aac_header_rules_t& GetAacHeaderRules();
	void SetAacHeaderRules(const aac_header_rules_t& aac_header_rules);

	// MediaPacketizer interface
protected:
	virtual std::size_t internal_push(const void* data, std::size_t size) override;
	virtual std::size_t internal_pop(void* data, std::size_t size) override;
	virtual void internal_reset() override;
	virtual std::size_t internal_count() const override;


};

} // audio

} // codec

} // largo

#endif // AAC_AUDIO_PACKETIZER_H
