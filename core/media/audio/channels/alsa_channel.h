#ifndef ALSA_CHANNEL_H
#define ALSA_CHANNEL_H

#include <string>
#include <vector>

#include "media/audio/channels/audio_channel_params.h"

namespace core
{

namespace media
{

namespace audio
{

namespace channels
{

#ifndef __ALSA_PCM_H
struct snd_pcm_t;
#endif


struct alsa_channel_info
{
	std::string name;
	std::string description;
	std::string hint;
	bool        input;
	bool        output;
};

class AlsaChannel
{
public:

	using device_names_list_t = std::vector<alsa_channel_info>;
	using sample_buffer_t = std::vector<std::uint8_t>;

private:

	std::string                     m_device_name;

	snd_pcm_t*                      m_handle;

	audio_channel_params_t			m_audio_params;

	std::uint32_t                   m_volume;

	sample_buffer_t                 m_sample_buffer;


public:

	AlsaChannel();
	~AlsaChannel();

	static const device_names_list_t GetDeviceList(bool recorder, const std::string& hw_profile = "");

	bool Open(const std::string& device_name, const audio_channel_params_t& audio_params = null_audio_params);
	bool Close();

	bool IsOpen() const;
	bool IsRecorder() const;

	inline const audio_channel_params_t& GetParams() const;
	bool SetParams(const audio_channel_params_t& audio_params);

	std::int32_t Read(void* capture_data, std::size_t size);
	std::int32_t Write(const void* playback_data, std::size_t size);

	inline void SetVolume(std::uint32_t volume) { m_volume = volume; }
	inline std::uint32_t GetVolume() const { return m_volume; }

private:

	std::int32_t setHardwareParams(const audio_channel_params_t& audio_params);

	std::int32_t internalRead(void* capture_data, std::size_t size);
	std::int32_t internalWrite(const void* playback_data, std::size_t size);

};

} // channels

} // audio

} // media

} // core


#endif // ALSA_DEVICE_H
