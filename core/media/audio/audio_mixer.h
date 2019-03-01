#ifndef AUDIO_MIXER_H
#define AUDIO_MIXER_H

#include "media/audio/audio_format.h"
#include "media/audio/i_audio_mixer.h"

namespace core
{

namespace media
{

namespace audio
{


class AudioMixer : public IAudioMixer
{
	audio_format_t		m_audio_format;

	bool				m_is_mixed_mode;

public:
	static std::size_t Mixed(const audio_format_t& audio_format,
													 std::size_t stream_count,
													 const void* input_data, std::size_t input_data_size,
													 const void* mixed_data, std::size_t mixed_data_size,
													 void* output_data, std::size_t output_data_size = 0);
	static std::size_t Mixed(const audio_format_t& audio_format,
													 std::size_t stream_count,
													 const void* input_data, std::size_t input_data_size,
													 void* mixed_data, std::size_t mixed_data_size);
	static std::size_t Demixed(const audio_format_t& audio_format,
													 std::size_t stream_count,
													 const void* input_data, std::size_t input_data_size,
													 const void* mixed_data, std::size_t mixed_data_size,
													 void* output_data, std::size_t output_data_size = 0);
	static std::size_t Demixed(const audio_format_t& audio_format,
													 std::size_t stream_count,
													 const void* input_data, std::size_t input_data_size,
													 void* mixed_data, std::size_t mixed_data_size);

	AudioMixer(const audio_format_t& audio_format, bool is_mixed_mode = true);
	~AudioMixer() override = default;

	std::size_t operator()(std::size_t stream_count,
						   const void* input_data, std::size_t input_data_size,
						   const void* mixed_data, std::size_t mixed_data_size,
						   void* output_data, std::size_t output_data_size = 0) override;

	std::size_t operator()(std::size_t stream_count,
						   const void* input_data, std::size_t input_data_size,
						   void* mixed_data, std::size_t mixed_data_size = 0) override;

	const audio_format_t& GetAudioFormat() const override;
	bool SetAudioFormat(const audio_format_t& audio_format) override;

	void SetMixedMode(bool mixed) override;
	bool IsMixedMode() const override;

};

} // audio

} // media

} // core

#endif // AUDIO_MIXER_H