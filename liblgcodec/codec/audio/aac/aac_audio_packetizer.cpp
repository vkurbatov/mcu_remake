#include "aac_audio_packetizer.h"
#include "common/bit_stream.h"

namespace largo
{

namespace codec
{

namespace audio
{

AacAudioPacketizer::AacAudioPacketizer(bool is_packetizer, const aac_header_rules_t& aac_header_rules)
	: MediaPacketizer(is_packetizer)
	, m_au_packetizer(aac_header_rules)
{

}

const aac_header_rules_t &AacAudioPacketizer::GetAacHeaderRules()
{
	m_au_packetizer.GetRules();
}

void AacAudioPacketizer::SetAacHeaderRules(const aac_header_rules_t &aac_header_rules)
{
	m_au_packetizer.SetRules(aac_header_rules);
}

std::size_t AacAudioPacketizer::internal_push(const void *data, std::size_t size)
{
	auto push_method = m_is_packetizer ? &AuPacketizer::PushFrame : &AuPacketizer::PushPacket;

	return (m_au_packetizer.*push_method)(data, size);
}

std::size_t AacAudioPacketizer::internal_pop(void *data, std::size_t size)
{
	auto pop_method = m_is_packetizer ? &AuPacketizer::PopPacket : &AuPacketizer::PopFrame;

	return (m_au_packetizer.*pop_method)(data, size);
}

void AacAudioPacketizer::internal_reset()
{
	m_au_packetizer.Clear();
}

std::size_t AacAudioPacketizer::internal_count() const
{
	return m_au_packetizer.Count();
}

} // audio

} // codec

} // largo
