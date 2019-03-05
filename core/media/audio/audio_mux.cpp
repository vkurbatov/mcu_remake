#include "audio_mux.h"
#include "media/audio/audio_mixer.h"

namespace core
{

namespace media
{

namespace audio
{

AudioMux::AudioMux(IAudioWriter& audio_writer
				   , IAudioReader& audio_reader)
	: m_audio_writer(audio_writer)
	, m_audio_reader(audio_reader)
{

}

IVolumeController& AudioMux::GetMainVolumeController()
{
	return m_main_volume_controller;
}

IVolumeController& AudioMux::GetAuxVolumeController()
{
	return m_aux_volume_controller;
}

std::int32_t AudioMux::Write(const audio_format_t& audio_format, const void* data, std::size_t size, std::uint32_t options)
{
	std::int32_t result = 0;


	if (size > 0)
	{
		if (size > m_aux_input_buffer.size())
		{
			m_aux_input_buffer.resize(size);
			m_main_input_buffer.resize(size);
		}

		auto mix_size = m_audio_reader.Read(audio_format, m_aux_input_buffer.data(), size);

		m_main_volume_controller(audio_format.sample_format, data, size, m_main_input_buffer.data());
		m_aux_volume_controller(audio_format.sample_format, m_aux_input_buffer.data(), mix_size);

		size = AudioMixer::Mixed(audio_format, 2, m_main_input_buffer.data(), size, m_aux_input_buffer.data(), mix_size, m_aux_input_buffer.data(), size);

		if (size > 0)
		{
			result = m_audio_writer.Write(audio_format, m_aux_input_buffer.data(), size);
		}
	}

	return result;
}

bool AudioMux::CanWrite() const
{
	return m_audio_writer.CanWrite();
}

} // audio

} // media

} // core
