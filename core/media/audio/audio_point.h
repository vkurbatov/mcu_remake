#ifndef AUDIO_POINT_H
#define AUDIO_POINT_H

#include "media/audio/audio_format.h"
#include "media/audio/i_audio_point.h"
#include "media/audio/volume_controller.h"
#include "media/common/media_point.h"
#include "media/audio/audio_resampler.h"

namespace core
{

namespace media
{

namespace audio
{

class AudioPoint : public MediaPoint, public IAudioPoint, public IAudioFormatter, public IVolumeController
{

	using audio_buffer_t = media_buffer_t;

	AudioResampler		m_input_resampler;
	AudioResampler		m_output_resampler;

	audio_buffer_t		m_input_resampler_buffer;

	audio_buffer_t		m_output_volume_buffer;

	VolumeController	m_volume_controller;


public:

	AudioPoint(const audio_format_t& input_format, const audio_format_t& output_format);
	virtual ~AudioPoint() override = default;

	// IAudioPoint interface
public:
	virtual std::int32_t Write(const void* data, std::size_t size, const audio_format_t& audio_format, std::uint32_t options = 0) override;
	virtual std::int32_t Read(void* data, std::size_t size, const audio_format_t& audio_format, std::uint32_t options = 0) override;


	// IAudioFormatter interface
public:
	virtual const audio_format_t& GetInputFormat() const override;
	virtual const audio_format_t& GetOutputFormat() const override;

	virtual void SetInputFormat(const audio_format_t& input_fromat) override;
	virtual void SetOutputFormat(const audio_format_t& output_fromat) override;

	// IVolumeController interface
public:

	virtual std::uint32_t GetVolume() const override;
	virtual void SetVolume(std::uint32_t volume) override;

	virtual bool IsMute() const override;
	virtual void SetMute(bool mute) override;

	// MediaPoint interface
public:
	virtual std::int32_t Write(const void* data, std::size_t size, std::uint32_t flags = 0) override final;
	virtual std::int32_t Read(void* data, std::size_t size, std::uint32_t flags = 0) override final;

};

} // audio

} // media

} // core


#endif // AUDIO_POINT_H
