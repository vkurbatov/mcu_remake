#ifndef ALSA_CHANNEL_H
#define ALSA_CHANNEL_H

#include <vector>

#include "media/audio/channels/audio_channel.h"

namespace core
{

namespace media
{

namespace audio
{

namespace channels
{

namespace alsa
{

#ifndef __ALSA_PCM_H
struct snd_pcm_t;
#endif


struct alsa_channel_info
{
	std::string		name;
	std::string		card_name;
	std::string		device_name;
	std::string		hint;
	bool			input;
	bool			output;

	const std::string user_format() const
	{
        return name == "default" ? name : (card_name == device_name ? card_name : card_name + "[" + device_name + "]");
	}
};

class AlsaChannel : public AudioChannel
{
public:

	using device_names_list_t = std::vector<alsa_channel_info>;
	using sample_buffer_t = std::vector<std::uint8_t>;

private:

	std::string                     m_device_name;
	std::string						m_hw_profile;

	snd_pcm_t*                      m_handle;

	audio_channel_params_t			m_audio_params;

	std::uint32_t					m_write_transaction_id;
	std::uint32_t					m_read_transaction_id;

	std::size_t						m_frame_size;


public:

	static const device_names_list_t GetDeviceList(channel_direction_t direction, const std::string& hw_profile = "plughw");

	AlsaChannel(const audio_channel_params_t& audio_params, const std::string& hw_profile = "plughw");
	~AlsaChannel() override;

	// IAudoChannel interface
public:
	bool Open(const std::string& device_name) override;
	bool Close() override;
	bool IsOpen() const override;

	bool IsRecorder() const override;
	bool IsPlayback() const override;

	const std::string &GetName() const override;

	// MediaPoint interface
public:
	bool CanRead() const override;
	bool CanWrite() const override;

	// AudioChannel interface
private:
	const audio_channel_params_t& internal_get_audio_params() const override final;
	bool internal_set_audio_params(const audio_channel_params_t& audio_params) override final;

	// MediaPoint interface
private:
	std::int32_t internal_read(void* data, std::size_t size, std::uint32_t options = 0) override final;
	std::int32_t internal_write(const void* data, std::size_t size, std::uint32_t options = 0) override final;

private:
	std::int32_t io_error_process(std::int32_t error, bool is_write, std::uint32_t timeout_ms = 0);
	std::int32_t set_hardware_params(const audio_channel_params_t& audio_params);
};

} // alsa

} // channels

} // audio

} // media

} // core


#endif // ALSA_DEVICE_H
