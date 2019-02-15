#define ALSA_PCM_NEW_HW_PARAMS_API 1

extern "C"
{
#include <alsa/asoundlib.h>
}

#include "alsa_channel.h"

#include <cstring>
#include <algorithm>

#include <core-tools/logging.h>

namespace core
{

namespace media
{

namespace audio
{

namespace channels
{

const char* device_info_fields[] = {"NAME", "DESC",  "IOID" };
const char default_hw_profile[] = "plughw:";
const char default_device_name[] = "default";
const std::int32_t default_max_io_retry_count = 5;
const std::int32_t default_max_set_hw_retry_count = 10;
const std::uint32_t max_volume = 100u;
const std::uint32_t min_volume = 0u;

namespace alsa_utils
{

static std::string get_field_from_hint(const void* hint, const char* field_name)
{
	std::string result;

	auto filed_value = snd_device_name_get_hint(hint, field_name);

	if (filed_value != nullptr)
	{

		result = filed_value;

		if (std::strcmp(filed_value, "null") != 0)
		{
			free(filed_value);
		}
	}

	return std::move(result);
}

snd_pcm_format_t bits_to_snd_format(std::uint32_t bits)
{
	snd_pcm_format_t result = SND_PCM_FORMAT_UNKNOWN;

	switch(bits)
	{
		case 8:
			result = SND_PCM_FORMAT_U8;
			break;
		case 16:
			result = SND_PCM_FORMAT_S16_LE;
			break;
		case 32:
			result = SND_PCM_FORMAT_S32_LE;
	}

	return result;
}

template<typename Tval>
void change_volume(const void *sound_data, std::size_t size, void *output_data, std::uint32_t volume)
{
	volume = std::min(max_volume, std::max(min_volume, volume));

	for (int i = 0; i < size / sizeof(Tval); i++)
	{

		auto& input_sample = *(static_cast<const Tval*>(sound_data) + i);
		auto& output_sample = *(static_cast<Tval*>(output_data) + i);

		output_sample = static_cast<Tval>((static_cast<double>(input_sample) * static_cast<double>(volume)) / 100.0f);

	}
}

void change_volume(const void *sound_data, std::size_t size, void* output_data, std::uint32_t bit_per_sample, std::uint32_t volume)
{
	switch(bit_per_sample)
	{
		case 8:
			change_volume<std::int8_t>(sound_data, size, output_data, volume);
			break;
		case 16:
			change_volume<std::int16_t>(sound_data, size, output_data, volume);
			break;
		case 32:
			change_volume<std::int32_t>(sound_data, size, output_data, volume);
			break;
	}
}

} // alsa_utils

AlsaChannel::AlsaChannel()
	: m_handle(nullptr)
	, m_device_name("default")
	, m_volume(100)
{

}

AlsaChannel::~AlsaChannel()
{
	Close();
}

const AlsaChannel::device_names_list_t AlsaChannel::GetDeviceList(bool recorder, const std::string &hw_profile)
{
	char ** hints = nullptr;

	device_names_list_t device_list;

	auto result = snd_device_name_hint(-1, "pcm", (void ***)&hints);

	if (result >=  0)
	{
		auto it = hints;

		while(*it != nullptr)
		{
			alsa_channel_info device_info = { "", "", "", false, false };

			enum fields_enum_t : std::uint32_t { name, desc, ioid };

			std::uint32_t i = 0;

			bool append = false;

			for (const auto f : device_info_fields)
			{
				auto field_value = alsa_utils::get_field_from_hint(*it, f);

				if ( field_value != "null" )
				{
					auto field_id = static_cast<fields_enum_t>(i++);

					if (!field_value.empty() || field_id == fields_enum_t::ioid)
					{
						switch(field_id)
						{
							case fields_enum_t::name:

								append = (field_value == default_device_name)
										|| (hw_profile.empty())
										|| (field_value.find(hw_profile) == 0);

								if (append == true)
								{
									device_info.name = field_value;
								}
								break;

							case fields_enum_t::desc:
							{
								auto delimeter_pos = field_value.find('\n');
								if (delimeter_pos != std::string::npos)
								{
									device_info.description = field_value.substr(0, delimeter_pos);
									device_info.hint = field_value.substr(delimeter_pos + 1);
								}
								else
								{
									device_info.description = device_info.name;
									device_info.hint = field_value;
								}

							}
								break;

							case fields_enum_t::ioid:

								device_info.input = field_value.empty() || field_value == "Input";
								device_info.output = field_value.empty() || field_value == "Output";
								append = recorder ? device_info.input : device_info.output;
						}

					}

					if (append == false)
					{
						break;
					}
				}

			}//foreach fields
			it++;

			if (append)
			{
				device_list.emplace_back(device_info);
			}

		}

		snd_device_name_free_hint((void**)hints);
	}

	return std::move(device_list);
}

bool AlsaChannel::Open(const std::string &device_name, const audio_channel_params_t& audio_params)
{
	bool result = false;
	if ( IsOpen() )
	{
		Close();
	}

	if ( audio_params.is_init() )
	{

		auto err = snd_pcm_open(&m_handle
								, device_name.c_str()
								, IsRecorder() ? SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK
												 , SND_PCM_NONBLOCK);
		if (err >= 0)
		{
			// snd_pcm_nonblock(m_handle, 0);
			result = setHardwareParams(audio_params) >= 0;

			if (result == false)
			{
				Close();
				LOG(warning) << "Can't Open device [" << device_name << "]: error set hardware params" LOG_END;
			}
			else
			{
				m_audio_params = audio_params;
				LOG(info) "Open device [" << device_name << "]: success" LOG_END;
			}
		}
		else
		{
			LOG(warning) << "Can't Open device [" << device_name << "]: errno = " << errno LOG_END;
		}
	}
	else
	{
		LOG(warning) << "Can't Open device [" << device_name << "]: audio params not set" LOG_END;
	}

	return result;
}

bool AlsaChannel::Close()
{
	bool result = false;

	if (m_handle != nullptr)
	{
		snd_pcm_abort(m_handle);
		snd_pcm_close(m_handle);

		m_handle = nullptr;

		LOG(info) << "Device [" << m_device_name << "] closed" LOG_END;
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

const audio_channel_params_t &AlsaChannel::GetParams() const
{
	return m_audio_params;
}

bool AlsaChannel::SetParams(const audio_channel_params_t &audio_params)
{
	bool result = audio_params.is_init() && (!IsOpen() || setHardwareParams(audio_params) >= 0);

	if (result == true)
	{
		m_audio_params = audio_params;
	}
	else
	{
		LOG(info) << "Cant't set params for [" << m_device_name << "]" LOG_END;
	}

	return result;
}

std::int32_t AlsaChannel::Read(void *capture_data, std::size_t size)
{
	std::int32_t result = -EBADF;

	if ( IsOpen() )
	{
		result = -EACCES;

		if (IsRecorder())
		{
			result = internalRead(capture_data, size);
		}
	}

	return result;
}

std::int32_t AlsaChannel::Write(const void *playback_data, std::size_t size)
{
	std::int32_t result = -EBADF;

	if ( IsOpen() )
	{
		result = -EACCES;

		if (!IsRecorder())
		{
			result = internalWrite(playback_data, size);
		}
	}

	return result;
}

std::int32_t AlsaChannel::setHardwareParams(const audio_channel_params_t& audio_params)
{
	std::int32_t result = -EINVAL;

	if ( audio_params.is_init() )
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

				result = snd_pcm_hw_params_set_format(m_handle, hw_params, alsa_utils::bits_to_snd_format(audio_params.audio_format.bit_per_sample));
				if (result < 0)
				{
					LOG(error) << "Can't set format S" << audio_params.audio_format.bit_per_sample << " hardware params, errno = " << result LOG_END;
					break;
				}

				result = snd_pcm_hw_params_set_channels(m_handle, hw_params, audio_params.audio_format.channels);
				if (result < 0)
				{
					LOG(error) << "Can't set channels " << audio_params.audio_format.channels << " hardware params, errno = " << result LOG_END;
					break;
				}

				auto sample_rate = audio_params.audio_format.sample_rate;
				result = snd_pcm_hw_params_set_rate_near(m_handle, hw_params, &sample_rate, nullptr);
				if (result < 0)
				{
					LOG(error) << "Can't set sample rate " << sample_rate << " hardware params, errno = " << result LOG_END;
					break;
				}

				//default buffer_size
				if(audio_params.buffer_size != 0)
				{
					snd_pcm_uframes_t period_size = (audio_params.buffer_size * audio_params.audio_format.bit_per_sample) / 8;
					result = snd_pcm_hw_params_set_buffer_size_near(m_handle, hw_params, &period_size);

					if (result < 0)
					{
						LOG(error) << "Can't set buffer size " << period_size << " hardware params, errno = " << result LOG_END;
						break;
					}

					period_size = audio_params.buffer_size;
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
				while(result == -EAGAIN || max_try-- > 0);

				if(result < 0)
				{
					LOG(error) << "Can't set hardware params, errno = " << errno LOG_END;
					break;
				}

				result = snd_pcm_nonblock(m_handle, static_cast<int>(audio_params.nonblock_mode));
				if(result < 0)
				{
					LOG(error) << "Can't set " << (audio_params.nonblock_mode ? "nonblock" : "block") << " mode, errno = " << result LOG_END;
					break;
				}

			}
			while(false);

		}

		if (result < 0)
		{
			result = -errno;
		}
		else
		{
			LOG(info) << "Set hardware params success " << errno LOG_END;
		}
	}

	return result;
}

std::int32_t AlsaChannel::internalRead(void *capture_data, std::size_t size)
{
	std::int32_t result = -1, total = 0;

	auto data = static_cast<std::int8_t*>(capture_data);

	auto frame_bytes = m_audio_params.audio_format.sample_octets();

	std::int32_t retry_read_count = 0;
	bool io_complete = false;

	do
	{
		// в начале каждого цикла априори снимаем признак завершения и
		// инкрементируем счетчик попыток ввода-вывода. Он все равно
		// сбросится при успешной операции ввода-вывода

		io_complete = false;
		retry_read_count++;

		auto err = snd_pcm_readi(m_handle, data, size / frame_bytes /*std::min(size / frame_bytes, 16ul)*/);

		switch(err)
		{
			case -EPIPE:
				snd_pcm_prepare(m_handle);
				break;

			case -ESTRPIPE:
			case -EAGAIN:

				// сюда попадаем если устройство не готово, snd_pcm_wait вернет 1,
				// если устройство освободилось за заданный таймаут в мсек

				if ( snd_pcm_wait( m_handle, m_audio_params.audio_format.duration_ms(size) ) != 1 )
				{
					io_complete = true;
				}

				break;

			default:
			{
				if (err > 0)
				{
					size -= err * frame_bytes;
					data += err * frame_bytes;
					total += err * frame_bytes;

					retry_read_count = 0;
				}
				else
				{
					// оставшиеся ошибки считаем фатальными

					io_complete = true;
				}
			}
		}

		if (err < 0)
		{
			LOG(error) << "Can't read alsa device with error = " << err << ", retry = " << retry_read_count LOG_END;
		}

		if ((io_complete |= (size == 0) || (retry_read_count >= default_max_io_retry_count)) == true)
		{
			result = err <= 0 ? err : total;
		}
	}
	while(!io_complete);

	if (result >= 0)
	{
		alsa_utils::change_volume(capture_data, size, capture_data, m_audio_params.audio_format.bit_per_sample, m_volume);
		// LOG(debug) << "Read " << total << " bytes from device success" LOG_END;
	}
	else
	{
		LOG(error) << "Failure read from device, errno = " << result LOG_END;
	}
	return result;
}

std::int32_t AlsaChannel::internalWrite(const void *playback_data, std::size_t size)
{
	std::int32_t result = 0, total = 0;

	auto frame_bytes = m_audio_params.audio_format.sample_octets();

	static auto trans_id = 0;

	std::int32_t retry_write_count = 0;
	bool io_complete = false;

	m_sample_buffer.resize(size);

	alsa_utils::change_volume(playback_data, size, m_sample_buffer.data(), m_audio_params.audio_format.bit_per_sample, m_volume);

	auto data = m_sample_buffer.data();

	do
	{
		// в начале каждого цикла априори снимаем признак завершения и
		// инкрементируем счетчик попыток ввода-вывода. Он все равно
		// сбросится при успешной операции ввода-вывода

		io_complete = false;
		retry_write_count++;

		auto err = snd_pcm_writei(m_handle, data, size / (frame_bytes * 2));

		switch(err)
		{
			case -EPIPE:
				snd_pcm_prepare(m_handle);
				break;

			case -ESTRPIPE:
			case -EAGAIN:

				// сюда попадаем если устройство не готово, snd_pcm_wait вернет 1,
				// если устройство освободилось за заданный таймаут в мсек

				if ( snd_pcm_wait( m_handle, m_audio_params.audio_format.duration_ms(size) ) != 1 )
				{
					io_complete = true;
				}

				break;

			default:
			{
				if (err > 0)
				{
					size -= err * frame_bytes;
					data += err * frame_bytes;
					total += err * frame_bytes;

					retry_write_count = 0;
				}
				else
				{
					// оставшиеся ошибки считаем фатальными

					io_complete = true;
				}
			}
		}

		if (err < 0)
		{
			LOG(error) << "Can't write alsa device with error = " << err << ", retry = " << retry_write_count << ", id = " << trans_id LOG_END;
		}

		if ((io_complete |= (size == 0) || (retry_write_count >= default_max_io_retry_count)) == true)
		{
			result = err <= 0 ? err : total;
		}
	}
	while(!io_complete);

	if (result >= 0)
	{
		// LOG(debug) << "Write " << total << " bytes from device success" LOG_END;
	}
	else
	{
		LOG(error) << "Failure write to device, errno = " << result LOG_END;
	}

	trans_id++;

	return result;
}

} // channels

} // audio

} // media

} // core
