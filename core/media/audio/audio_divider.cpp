#include "audio_divider.h"

namespace core
{

namespace media
{

namespace audio
{

core::media::audio::AudioDivider::AudioDivider(IAudioWriter& audio_writer1
											   , IAudioWriter& audio_writer2)
	: m_audio_writer1(audio_writer1)
	, m_audio_writer2(audio_writer2)
{

}

std::int32_t core::media::audio::AudioDivider::Write(const audio_format_t& audio_format, const void* data, std::size_t size, std::uint32_t options)
{
	return std::max(
				m_audio_writer1.Write(audio_format, data, size, options)
				, m_audio_writer2.Write(audio_format, data, size, options)
				);
}


} // audio

} // media

} // core
