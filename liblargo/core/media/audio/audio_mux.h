#ifndef AUDIO_MUX_H
#define AUDIO_MUX_H

#include "core/media/audio/i_audio_point.h"
#include "core/media/audio/i_audio_formatter.h"
#include "core/media/audio/volume_controller.h"

namespace core
{

namespace media
{

namespace audio
{

const std::uint32_t default_mux_queue_size_ms = 1000;

class AudioMux: public IAudioWriter
{

	VolumeController	m_main_volume_controller;
	VolumeController	m_aux_volume_controller;

	media_buffer_t		m_main_input_buffer;
	media_buffer_t		m_aux_input_buffer;

	IAudioReader&		m_audio_reader;
	IAudioWriter&		m_audio_writer;


public:
	AudioMux(IAudioWriter& audio_writer, IAudioReader& audio_reader);
	IVolumeController& GetMainVolumeController();
	IVolumeController& GetAuxVolumeController();

	// IAudioWriter interface
public:
	std::int32_t Write(const audio_format_t& audio_format, const void* data, std::size_t size, uint32_t options) override;

	// IMediaWriteStatus interface
public:
	bool CanWrite() const override;
};

} // audio

} // media

} // core

#endif // AUDIO_MUX_H
