#ifndef AUDIO_POINT_H
#define AUDIO_POINT_H

#include "audio_format.h"
#include "media/common/media_point.h"
#include "media/audio/audio_formatter.h"

#include <vector>

namespace core
{

namespace media
{

namespace audio
{

class AudioPoint : public common::MediaPoint, public AudioFormatter
{

	using audio_buffer_t = std::vector<std::uint8_t>;

	audio_buffer_t	m_input_resample_buffer;
	audio_buffer_t	m_output_resample_buffer;

public:

    AudioPoint(const audio_format_t& input_format, const audio_format_t& output_format);
    virtual ~AudioPoint() override = default;

    virtual std::int32_t Write(const void* data, std::size_t size, const audio_format_t& audio_format, std::uint32_t flags = 0);
    virtual std::int32_t Read(void* data, std::size_t size, const audio_format_t& audio_format, std::uint32_t flags = 0);

private:

	void* getResampleBuffer(bool input, std::size_t size);

};

} // audio

} // media

} // core


#endif // AUDIO_POINT_H
