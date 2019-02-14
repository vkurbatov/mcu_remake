#ifndef AUDIO_POINT_H
#define AUDIO_POINT_H

#include "audio_format.h"
#include "media/common/media_point.h"
#include "media/audio/audio_resampler.h"

namespace core
{

namespace media
{

namespace audio
{

class AudioPoint : public MediaPoint, public IAudioFormatter
{

	using audio_buffer_t = media_buffer_t;

	AudioResampler	m_audio_resampler;
	audio_buffer_t	m_output_resampler_buffer;


public:

    AudioPoint(const audio_format_t& input_format, const audio_format_t& output_format);
    virtual ~AudioPoint() override = default;

	virtual std::int32_t Write(const void* data, std::size_t size, const audio_format_t& audio_format, std::uint32_t volume = 100u, std::uint32_t flags = 0);
	virtual std::int32_t Read(void* data, std::size_t size, const audio_format_t& audio_format, std::uint32_t volume = 100u, std::uint32_t flags = 0);


	// IAudioFormatter interface
public:
	virtual const audio_format_t& GetInputFormat() const override;
	virtual const audio_format_t& GetOutputFormat() const override;

	virtual void SetInputFormat(const audio_format_t& input_fromat) override;
	virtual void SetOutputFormat(const audio_format_t& output_fromat) override;
};

} // audio

} // media

} // core


#endif // AUDIO_POINT_H
