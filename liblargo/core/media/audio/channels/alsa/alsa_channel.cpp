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

const std::uint32_t default_period_size = 4096;
const std::uint32_t default_periods = 4;
const std::uint32_t default_buffer_size = default_period_size * default_periods;
const std::size_t   asla_min_period_size = 32;

const char default_hw_profile[] = "plughw:";
const std::int32_t default_max_io_retry_count = 5;
const std::int32_t default_max_set_hw_retry_count = 10;

namespace alsa_utils
{

snd_pcm_format_t snd_format_from_sample_format(sample_format_t sample_format)
{

	const static snd_pcm_format_t transform_format_array[] =
	{
		SND_PCM_FORMAT_UNKNOWN,
		SND_PCM_FORMAT_S8,
		SND_PCM_FORMAT_S16,
		SND_PCM_FORMAT_S32,
		SND_PCM_FORMAT_FLOAT,
		SND_PCM_FORMAT_FLOAT64
	};

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
	static alsa_device_list_t	recorder_device_list;
	static alsa_device_list_t	playback_device_list;

	alsa_device_list_t& device_list = is_recorder ? recorder_device_list : playback_device_list;

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

		while (snd_card_next(&card_number) >= 0 && card_number >= 0)
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
	, m_hw_profile(hw_profile)
	, m_resume_support(true)
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

	if (IsOpen())
	{
		Close();
	}

	if (m_audio_params.is_valid())
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

				if (IsPlayback())
				{
					snd_pcm_prepare(m_handle);
				}

				m_resume_support = true;

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

		// usleep(1000000);

		m_handle = nullptr;

		LOG(info) << "Device \'" << m_device_name << "\' closed" LOG_END;

		result = true;
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

	std::int32_t retry_read_count = 0;
	bool io_complete = false;

	auto frame_size = get_frame_size(std::max(size, sample_size));

	do
	{
		if (m_handle == nullptr)
		{
			break;
		}

		io_complete = false;
		retry_read_count++;

		auto part_size = std::min(size, frame_size) / (sample_size);

		auto err = snd_pcm_readi(m_handle, data_ptr, part_size);

		if (err == -EAGAIN)
		{
			auto wait_timeout = std::max(m_audio_params.wait_timeout_ms, audio_format.duration_ms(part_size));
			snd_pcm_wait(m_handle, wait_timeout);
		}
		else if (err >= 0)
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
	while (!io_complete);

	if (result >= 0)
	{
		// LOG ???
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

	auto data_ptr = static_cast<const std::uint8_t*>(data);

	auto frame_size = get_frame_size(std::max(size, sample_size));

	do
	{
		if (m_handle == nullptr)
		{
			break;
		}

		io_complete = false;
		retry_write_count++;

		auto part_size = std::min(size, frame_size) / (sample_size);

		auto err = snd_pcm_writei(m_handle, data_ptr, part_size);

		if (err == -EAGAIN)
		{
			auto wait_timeout = std::max(m_audio_params.wait_timeout_ms, audio_format.duration_ms(part_size));
			snd_pcm_wait(m_handle, wait_timeout);
		}
		else if (err >= 0)
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
			err = io_error_process(err, true, audio_format.duration_ms(size));
		}

		if (err < 0)
		{
			LOG(error) << "Can't write alsa device with error = " << err << ", retry = " << retry_write_count << ", id = " << m_write_transaction_id LOG_END;
		}

		io_complete |= (size == 0) || (retry_write_count >= default_max_io_retry_count);

		if (io_complete == true)
		{
			result = err < 0 ? err : total;
		}
	}
	while (!io_complete);

	if (result >= 0)
	{
		// LOG ???
	}
	else
	{
		LOG(error) << "Failure write to device, errno = " << result LOG_END;
	}

	m_write_transaction_id++;

	return result;
}

std::int32_t AlsaChannel::set_hardware_params(const audio_channel_params_t& audio_params)
{
	std::int32_t result = -EINVAL;

	if (audio_params.is_valid())
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


				// default buffer_size ?

				snd_pcm_uframes_t period_size = get_period_size();
				snd_pcm_uframes_t buffer_size = get_buffer_size();

				if (period_size != 0 && buffer_size != 0)
				{

					result = snd_pcm_hw_params_set_buffer_size_near(m_handle, hw_params, &buffer_size);

					if (result < 0)
					{
						LOG(error) << "Can't set buffer size " << period_size << " hardware params, errno = " << result LOG_END;
						break;
					}


					result = snd_pcm_hw_params_set_period_size_near(m_handle, hw_params, &period_size, nullptr);

					if (result < 0)
					{
						LOG(error) << "Can't set period size " << period_size << " hardware params, errno = " << result LOG_END;
						break;
					}

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
				while ((result == -EAGAIN) || (max_try-- > 0));

				if (result < 0)
				{
					LOG(error) << "Can't set hardware params, errno = " << result LOG_END;
					break;
				}

				result = snd_pcm_nonblock(m_handle, static_cast<std::int32_t>(audio_params.is_nonblock()));

				if (result < 0)
				{
					LOG(error) << "Can't set " << (audio_params.is_nonblock() ? "nonblock" : "block") << " mode, errno = " << result LOG_END;
					break;
				}

			}
			while (false);

		}

		if (result >= 0)
		{
			result = snd_pcm_prepare(m_handle);
			LOG(info) << "Set hardware params success " << result LOG_END;
		}
	}

	return result;
}

int32_t AlsaChannel::io_error_process(int32_t error, bool is_write, std::uint32_t timeout_ms)
{

	if (timeout_ms == 0)
	{
		timeout_ms = 0xffffffff;
	}

#define string_type(is_write) (is_write ? "write" : "read")

	switch (error)
	{
	case -EPIPE:

		LOG(debug) << "IO Error[" << string_type(is_write) <<  "]: broken pipe. Need prepare device." LOG_END;

		if (is_write)
		{
			error = prepare_playback();
		}
		else
		{
			error = snd_pcm_prepare(m_handle);
		}

		break;

	case -EBADFD:

		LOG(error) << "IO Error[" << string_type(is_write) <<  "]: bad faile descriptor. Need prepare device???" LOG_END;
		error = snd_pcm_prepare(m_handle);

		break;

	case -ESTRPIPE:
	{
		if (m_resume_support)
		{
			while (((error = snd_pcm_resume(m_handle)) == -EAGAIN) && (timeout_ms != 0))
			{
				usleep(1000); // 1 msec
				timeout_ms -= 1;
			}

			m_resume_support &= error != -ENOSYS;
		}

		if (error < 0)
		{
			LOG(debug) << "IO Error[" << string_type(is_write) <<  "]: Unblock IO failed(" << error << "). Need prepare device." LOG_END;
			error = snd_pcm_prepare(m_handle);
		}
	}
	break;
	}

	return error;
}

std::size_t AlsaChannel::get_frame_size(size_t size) const
{
	std::size_t result = m_audio_params.period_size();

	if (result == 0 || result > size)
	{
		result = size;
	}

	return result;
}

size_t core::media::audio::channels::alsa::AlsaChannel::get_buffer_size() const
{
	std::size_t buffer_size = m_audio_params.buffer_size();
	std::size_t period_size = m_audio_params.period_size();

	if (period_size != 0)
	{
		if (buffer_size == 0)
		{
			buffer_size = period_size * default_periods;
		}
		else
		{
			buffer_size = std::max(buffer_size, period_size * 2);
		}
	}

	return buffer_size;
}

size_t core::media::audio::channels::alsa::AlsaChannel::get_period_size() const
{
	std::size_t buffer_size = m_audio_params.buffer_size();
	std::size_t period_size = m_audio_params.period_size();

	if (buffer_size != 0)
	{
		if (period_size == 0)
		{
			period_size = buffer_size / default_periods;
		}
		else
		{
			period_size = std::max(period_size, asla_min_period_size);
		}
	}

	return period_size;
}

std::int32_t AlsaChannel::prepare_playback()
{
	std::int32_t result = 0;

	auto sample_size = m_audio_params.audio_format.bytes_per_sample();
	auto silense_buffer_size = get_buffer_size();

	result = snd_pcm_prepare(m_handle);

	if (result >= 0 && sample_size > 0)
	{
		if (silense_buffer_size > m_silense_buffer.size())
		{
			m_silense_buffer.resize(silense_buffer_size);
			std::memset(m_silense_buffer.data(), 0, silense_buffer_size);
		}

		LOG(warning) << "Playback write " << silense_buffer_size << " bytes silense data" LOG_END;

		result = snd_pcm_writei(m_handle, m_silense_buffer.data(), silense_buffer_size / sample_size);

		if (result == -EPIPE)
		{
			result = snd_pcm_prepare(m_handle);
		}
	}

	return result;
}

} // alsa

} // channels

} // audio

} // media

} // core
