#include "media/audio/audio_resampler.h"
#include "audio_slot.h"

namespace core
{

namespace media
{

namespace audio
{

AudioSlot::AudioSlot(const audio_format_t& audio_format, IMediaSlot& media_slot)
	: m_audio_format(audio_format)
	, m_media_slot(media_slot)
	, m_palyback_queue(media_slot.Capacity())
	, m_ref_count(1)
{

}

std::int32_t AudioSlot::Write(const void* data, std::size_t size, const audio_format_t& audio_format, std::uint32_t options)
{
	auto output_size = m_audio_format.size_from_format(audio_format, size);

	if (output_size > m_output_resampler_buffer.size())
	{
		m_output_resampler_buffer.resize(output_size);
	}

	output_size = AudioResampler::Resampling(audio_format, m_audio_format, data, size, m_output_resampler_buffer.data(), m_output_resampler_buffer.size());

	output_size = m_media_slot.Push(m_output_resampler_buffer.data(), output_size);

	return audio_format.size_from_format(m_audio_format, output_size);

}

std::int32_t AudioSlot::Read(void* data, std::size_t size, const audio_format_t& audio_format, std::uint32_t options)
{
	auto input_size = m_audio_format.size_from_format(audio_format, size);

	if (input_size > m_input_resampler_buffer.size())
	{
		m_input_resampler_buffer.resize(input_size);
	}

	input_size = m_media_slot.Pop(m_input_resampler_buffer.data(), input_size);

	input_size = AudioResampler::Resampling(audio_format, m_audio_format, m_output_resampler_buffer.data(), m_output_resampler_buffer.size(), data, size);

	return audio_format.size_from_format(m_audio_format, input_size);
}

audio_slot_id_t AudioSlot::GetSlotId() const
{
	return m_media_slot.GetSlotId();
}

bool AudioSlot::IsSkip() const
{
	return false;
}

} // audio

} // media

} // core
