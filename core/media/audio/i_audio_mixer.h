#ifndef I_AUDIO_MIXER_H
#define I_AUDIO_MIXER_H

#include <cstdint>

namespace core
{

namespace media
{

namespace audio
{

#ifndef AUDIO_FORMT_H
struct audio_format_t;
#endif

class IAudioMixer
{
public:
	virtual ~IAudioMixer() = default;

	virtual std::size_t operator()(std::size_t stream_count,
						   const void* input_data, std::size_t input_data_size,
						   const void* mixed_data, std::size_t mixed_data_size,
						   void* output_data, std::size_t output_data_size = 0) = 0;

	virtual std::size_t operator()(std::size_t stream_count,
						   const void* input_data, std::size_t input_data_size,
						   void* mixed_data, std::size_t mixed_data_size = 0) = 0;

	virtual const audio_format_t& GetAudioFormat() const = 0;
	virtual bool SetAudioFormat(const audio_format_t& audio_format) = 0;

	virtual void SetMixedMode(bool mixed) = 0;
	virtual bool IsMixedMode() const = 0;

};

} // audio

} // media

} // core

#endif // I_AUDIO_MIXER_H
