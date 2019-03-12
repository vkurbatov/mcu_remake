#define ALSA_PCM_NEW_HW_PARAMS_API 1

extern "C"
{
#include <alsa/asoundlib.h>
}

#include "alsa_channel.h"

#include <cstring>
#include <algorithm>

#include <core-tools/logging.h>
#include "core/media/audio/audio_string_format_utils.h"

#define PTraceModule() "alsa_channel"

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

// const char* device_info_fields[] = {"NAME", "DESC",  "IOID" };
const char default_hw_profile[] = "plughw:";
// const char default_device_name[] = "default";
const std::int32_t default_max_io_retry_count = 5;
const std::int32_t default_max_set_hw_retry_count = 10;

AlsaChannel::alsa_device_list_t AlsaChannel::m_recorder_device_list;
AlsaChannel::alsa_device_list_t AlsaChannel::m_playback_device_list;

namespace alsa_utils
{

snd_pcm_format_t snd_format_from_sample_format(audio_format_t::sample_format_t sample_format)
{

	const static snd_pcm_format_t transform_format_array[] = {
		SND_PCM_FORMAT_UNKNOWN,
		SND_PCM_FORMAT_S8,
		SND_PCM_FORMAT_S16,
		SND_PCM_FORMAT_S32,
		SND_PCM_FORMAT_FLOAT,
		SND_PCM_FORMAT_FLOAT64 };

	return transform_format_array[static_cast<std::int32_t>(sample_format)];
}

const std::string fetch_native_device_name(const std::string& device_name, bool is_recorder, const std::string& profile)
{
	std::string result = device_name;

	const auto& devices = AlsaChannel::GetDeviceList(is_recorder);

	auto it = std::find(devices.begin(), devices.end(), device_name);

	if (it != devices.end())
	{
		result = it->native_format(profile);
	}

	return result;
}

} // alsa_utils

const AlsaChannel::alsa_device_list_t& AlsaChannel::GetDeviceList(bool is_recorder, bool update)
{
	alsa_device_list_t& device_list = is_recorder ? m_recorder_device_list : m_playback_device_list;

	update |= device_list.size() == 0;

	if (update)
	{
		device_list.clear();

		device_list.emplace_back(is_recorder, -2, -1, "default", "default");

		std::int32_t card_number = -1;

		snd_ctl_card_info_t * info = NULL;
		snd_ctl_card_info_alloca(&info);

		snd_pcm_info_t * pcminfo = NULL;
		snd_pcm_info_alloca(&pcminfo);

		while (snd_card_next(&card_number) >=0 && card_number >=0 )
		{
			snd_ctl_t *ctl_handle = nullptr;
			std::string card_id = "hw:" + std::to_string(card_number);

			if (snd_ctl_open(&ctl_handle, card_id.c_str(), 0) >= 0)
			{
				snd_ctl_card_info(ctl_handle, info);

				std::int32_t device_number = -1;

				while (snd_ctl_pcm_next_device(ctl_handle, &device_number) >= 0 && device_number >= 0)
				{
					snd_pcm_info_set_device(pcminfo, device_number);
					snd_pcm_info_set_subdevice(pcminfo, 0);
					snd_pcm_info_set_stream(pcminfo, is_recorder ? SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK);

					if (snd_ctl_pcm_info(ctl_handle, pcminfo) >= 0)
					{
						char* card_name = nullptr;

						snd_card_get_name(card_number, &card_name);

						if (card_name != nullptr)
						{
							device_list.emplace_back(is_recorder
													 , card_number
													 , device_number
													 , card_name
													 , snd_pcm_info_get_name(pcminfo));

						}

					}
				}
			}
		}

	}

	return device_list;
}

AlsaChannel::AlsaChannel(const audio_channel_params_t& audio_params, const std::string& hw_profile)
	: m_audio_params(audio_params)
	, m_handle(nullptr)
	, m_device_name("default")
	, m_write_transaction_id(0)
	, m_read_transaction_id(0)
	, m_frame_size(0)
	, m_hw_profile(hw_profile)
{
	LOG(debug) << "Create alsa channel with params " << audio_params LOG_END;
}

AlsaChannel::~AlsaChannel()
{
	Close();
	LOG(debug) << "Destroy alsa channel " << m_audio_params LOG_END;
}

bool AlsaChannel::Open(const std::string &device_name)
{
	bool result = false;

	if ( IsOpen() )
	{
		Close();
	}

	if ( m_audio_params.is_valid() )
	{

		m_handle = nullptr;
		auto err = snd_pcm_open(&m_handle
								, alsa_utils::fetch_native_device_name(device_name, IsRecorder(), m_hw_profile).c_str()
								, IsRecorder() ? SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK
								, SND_PCM_NONBLOCK);
		if (err >= 0)
		{

			m_device_name = device_name;
			result = set_hardware_params(m_audio_params) >= 0;

			if (result == false)
			{
				LOG(error) << "Can't Open device \'" << device_name << "\': error set hardware params" LOG_END;
				Close();
			}
			else
			{
				m_write_transaction_id = m_read_transaction_id = 0;
				LOG(info) << "Open device \'" << device_name << "\': success" LOG_END;
			}
		}
		else
		{
			LOG(error) << "Can't Open device \'" << device_name << "\': errno = " << err LOG_END;
			m_handle = nullptr;
		}
	}
	else
	{
		LOG(warning) << "Can't Open device \'" << device_name << "\': audio params not set" LOG_END;
	}

	return result;
}

bool AlsaChannel::Close()
{
	bool result = false;

	if (m_handle != nullptr)
	{
        // snd_pcm_abort(m_handle);
		snd_pcm_close(m_handle);

		m_handle = nullptr;

		LOG(info) << "Device \'" << m_device_name << "\' closed" LOG_END;
	}

	return result;
}

bool AlsaChannel::IsOpen() const
{
	return m_handle != nullptr;
}

bool AlsaChannel::IsRecorder() const
{
	return m_audio_params.direction == channel_direction_t::recorder || m_audio_params.direction == channel_direction_t::both;
}

bool AlsaChannel::IsPlayback() const
{
	return m_audio_params.direction == channel_direction_t::playback || m_audio_params.direction == channel_direction_t::both;
}

const std::string& AlsaChannel::GetName() const
{
	return m_device_name;
}

bool AlsaChannel::CanRead() const
{
	return IsOpen() && IsRecorder();
}

bool AlsaChannel::CanWrite() const
{
	return IsOpen() && IsPlayback();
}

const audio_channel_params_t &AlsaChannel::internal_get_audio_params() const
{
	return m_audio_params;
}

bool AlsaChannel::internal_set_audio_params(const audio_channel_params_t &audio_params)
{
	bool result = audio_params.is_valid() && (!IsOpen() || set_hardware_params(audio_params) >= 0);

	if (result == true)
	{
		// always save ???
		m_audio_params = audio_params;
	}
	else
	{
		LOG(info) << "Cant't set params [" << audio_params << "] for [" << m_device_name << "]" LOG_END;
	}

	return result;
}

std::int32_t AlsaChannel::internal_read(void *data, std::size_t size, std::uint32_t options)
{
	std::int32_t result = -1, total = 0;

	auto data_ptr = static_cast<std::int8_t*>(data);

	const auto& audio_format = GetAudioFormat();

	size_t sample_size = audio_format.bytes_per_sample();

	auto errors = 0, last_error = 0;

	std::int32_t retry_read_count = 0;
	bool io_complete = false;

	do
	{

		io_complete = false;
		retry_read_count++;

		auto part_size = std::min(size, m_frame_size) / (sample_size);

		auto err = snd_pcm_readi(m_handle, data_ptr, part_size);

		if (err >= 0)
		{
			io_complete = (err == 0);

			if (io_complete == false)
			{
				size -= err * sample_size;
				data_ptr += err * sample_size;
				total += err * sample_size;

				retry_read_count = 0;
			}
		}
		else
		{
			last_error = err;
			errors++;
			err = io_error_process(err, false, audio_format.duration_ms(size));
		}

		if (err < 0)
		{
			LOG(error) << "Can't read alsa device with error = " << err << ", retry = " << retry_read_count << ", trans_id = " << m_read_transaction_id LOG_END;
		}

		io_complete |= (size == 0) || (retry_read_count >= default_max_io_retry_count);

		if (io_complete == true)
		{
			result = err < 0 ? err : total;
		}
	}
	while(!io_complete);

	if (result >= 0)
	{
		// LOG ???
		if (errors > 0)
		{
			LOG(debug) << "Success read " << result << " bytes, with " << errors << " errors. Last error = " << last_error << ". Frame size = " << m_frame_size LOG_END;
		}
	}
	else
	{
		LOG(error) << "Failure read from device, errno = " << result LOG_END;
	}

	m_read_transaction_id++;

	return result;
}

std::int32_t AlsaChannel::internal_write(const void *data, std::size_t size, std::uint32_t options)
{
	std::int32_t result = 0, total = 0;

	const auto& audio_format = GetAudioFormat();

	std::size_t sample_size = audio_format.bytes_per_sample();

	std::int32_t retry_write_count = 0;
	bool io_complete = false;

	auto errors = 0, last_error = 0;

	auto data_ptr = static_cast<const std::uint8_t*>(data);

	do
	{

		io_complete = false;
		retry_write_count++;

		auto part_size = std::min(size, m_frame_size) / (sample_size);

		auto err = snd_pcm_writei(m_handle, data_ptr, part_size);

		if (err >= 0)
		{
			io_complete = (err == 0);

			if (io_complete == false)
			{
				size -= err * sample_size;
				data_ptr += err * sample_size;
				total += err * sample_size;

				retry_write_count = 0;
			}
		}
		else
		{
			errors++;
			last_error = err;
			err = io_error_process(err, true, audio_format.duration_ms(size));
		}

		if (err < 0)
		{
			// LOG(error) << "Can't write alsa device with error = " << err << ", retry = " << retry_write_count << ", id = " << m_write_transaction_id LOG_END;
		}

		io_complete |= (size == 0) || (retry_write_count >= default_max_io_retry_count);

		if (io_complete == true)
		{
			result = err < 0 ? err : total;
		}
	}
	while(!io_complete);

	if (result >= 0)
	{
		if (errors > 0)
		{
			LOG(debug) << "Success write " << result << " bytes, with " << errors << " errors. Last error = " << last_error << ". Frame size = " << m_frame_size LOG_END;
		}
		// LOG ???
	}
	else
	{
		LOG(error) << "Failure write to device, errno = " << result LOG_END;
	}

	m_write_transaction_id++;

	return result;
}

int32_t AlsaChannel::io_error_process(int32_t error, bool is_write, std::uint32_t timeout_ms)
{

	// весь этот код нужно пересмотреть вокруг функции snd_pcm_recover,
	// и все лишнее выкинуть нахуй
	// При работке с альсой (ошибои ввода-вывода), было вычитано что код пост-обработки
	// ошибки должен быть резким, как понос с будуна. Убраны все логи!

	if (timeout_ms == 0)
	{
		timeout_ms = 0xffffffff;
	}

	// error = snd_pcm_recover(m_handle, error, -1);
	// snd_pcm_recover(m_handle, error, -1);

	#define string_type(is_write) (is_write ? "write" : "read")

	switch(error)
	{
		case -EPIPE:

			// LOG(error) << "IO Error[" << string_type(is_write) <<  "]: broken pipe. Need prepare device." LOG_END;

			if ( (error = snd_pcm_prepare(m_handle)) >= 0 )
			{			
				auto frame_size = std::max(m_frame_size - m_audio_params.audio_format.bytes_per_sample(), m_audio_params.audio_format.bytes_per_sample() * 2);

				if (frame_size < m_frame_size)
				{
					// LOG(info) << "Clamp frame_size from " << m_frame_size << " to " << frame_size LOG_END;
					m_frame_size = frame_size;
				}
			}

			break;
		case -EBADFD:

			// LOG(error) << "IO Error[" << string_type(is_write) <<  "]: bad faile descriptor. Need prepare device." LOG_END;
			error = snd_pcm_prepare(m_handle);

			break;
		case -ESTRPIPE:
		case -EAGAIN:
			{
				while ( ((error = snd_pcm_resume(m_handle)) == -EAGAIN) && (timeout_ms != 0) )
				{
					usleep(1000); // 1 msec
					timeout_ms -= 1;
				}

				if (error < 0)
				{
					// LOG(error) << "IO Error[" << string_type(is_write) <<  "]: Unblock IO failed(" << error << "). Need prepare device." LOG_END;
					error = snd_pcm_prepare(m_handle);
				}
			}
		break;
	}

	return error;
}

std::int32_t AlsaChannel::set_hardware_params(const audio_channel_params_t& audio_params)
{
	std::int32_t result = -EINVAL;

	if ( audio_params.is_valid() )
	{
		snd_pcm_hw_params_t* hw_params = nullptr;
		snd_pcm_hw_params_alloca(&hw_params);

		if (hw_params != nullptr)
		{
			// for breaking seq
			do
			{
				result = snd_pcm_hw_params_any(m_handle, hw_params);
				if (result < 0)
				{
					LOG(error) << "Can't init hardware params, errno = " << result LOG_END;
					break;
				}

				result = snd_pcm_hw_params_set_access(m_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
				if (result < 0)
				{
					LOG(error) << "Can't set access hardware params, errno = " << result LOG_END;
					break;
				}

				result = snd_pcm_hw_params_set_format(m_handle, hw_params, alsa_utils::snd_format_from_sample_format(audio_params.audio_format.sample_format));
				if (result < 0)
				{
					LOG(error) << "Can't set format S" << static_cast<std::int32_t>(audio_params.audio_format.sample_format) << " hardware params, errno = " << result LOG_END;
					break;
				}

				result = snd_pcm_hw_params_set_channels(m_handle, hw_params, audio_params.audio_format.channels);
				if (result < 0)
				{
					LOG(error) << "Can't set channels " << audio_params.audio_format.channels << " hardware params, errno = " << result LOG_END;
					break;
				}

				auto sample_rate = audio_params.audio_format.sample_rate;
				result = snd_pcm_hw_params_set_rate(m_handle, hw_params, sample_rate, 0);
				if (result < 0)
				{
					LOG(error) << "Can't set sample rate " << sample_rate << " hardware params, errno = " << result LOG_END;
					break;
				}

				std::uint32_t periods = 2;

				// default buffer_size ?
				if(audio_params.buffer_duration_ms != 0)
				{

					snd_pcm_uframes_t period_size = audio_params.buffer_size() * periods;

					result = snd_pcm_hw_params_set_periods_near(m_handle, hw_params, &periods, nullptr);
					if (result < 0)
					{
						LOG(error) << "Can't set periods " << periods << " hardware params, errno = " << result LOG_END;
						break;
					}

					result = snd_pcm_hw_params_set_period_size_near(m_handle, hw_params, &period_size, nullptr);
					if (result < 0)
					{
						LOG(error) << "Can't set period size " << period_size << " hardware params, errno = " << result LOG_END;
						break;
					}


					m_frame_size = audio_params.buffer_size() / 2;

				}

				std::int32_t max_try = default_max_set_hw_retry_count;

				do
				{
					result = snd_pcm_hw_params(m_handle, hw_params);

					if (result == -EAGAIN)
					{
						snd_pcm_wait(m_handle, 1);
					}
				}
				while((result == -EAGAIN) || (max_try-- > 0));

				if(result < 0)
				{
					LOG(error) << "Can't set hardware params, errno = " << result LOG_END;
					break;
				}

				result = snd_pcm_nonblock(m_handle, static_cast<std::int32_t>(audio_params.nonblock_mode));
				if(result < 0)
				{
					LOG(error) << "Can't set " << (audio_params.nonblock_mode ? "nonblock" : "block") << " mode, errno = " << result LOG_END;
					break;
				}

			}
			while(false);

		}

		if (result >= 0)
		{
			result = snd_pcm_prepare(m_handle);
			LOG(info) << "Set hardware params success " << result LOG_END;
		}
	}

	return result;
}

} // alsa

} // channels

} // audio

} // media

} // core
