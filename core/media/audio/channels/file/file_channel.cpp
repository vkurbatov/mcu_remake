#include "file_channel.h"
#include <cstring>

namespace core
{

namespace media
{

namespace audio
{

namespace channels
{

namespace file
{

namespace file_utils
{

#pragma pack(push,1)

struct wav_header_t
{
	char				chunk_id[4];			// RIFF Header Magic header
	std::uint32_t       chunk_size;				// RIFF Chunk Size
	char				format_id[4];			// WAVE Header
	char				sub_chunk_id[4];		// FMT header
	std::uint32_t       sub_chunk_size;			// Size of the fmt chunk
	std::uint16_t       audio_format;			// Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
	std::uint16_t       channels;				// Number of channels 1=Mono 2=Sterio
	std::uint32_t       sample_rate;			// Sampling Frequency in Hz
	std::uint32_t       byte_rate;				// bytes per second
	std::uint16_t       block_align;			// 2=16-bit mono, 4=16-bit stereo
	std::uint16_t       bit_per_sample;			// Number of bits per sample
	char				chunk_2_id[4];			// "data"  string or addition section
	std::uint32_t       chunk_2_size;			// Sampled data or size this chunk
};

#pragma pack(pop)

static const char wav_chunk_id[] = "RIFF";
static const char wav_format_id[] = "WAVE";
static const char wav_sub_chunk_id[] = "fmt ";
static const char wav_chunk_2_id[] = "data";

static const std::uint16_t wav_format_pcm = 1;
static const std::uint16_t wav_format_mulaw = 6;
static const std::uint16_t wav_format_alaw = 7;
static const std::uint16_t wav_format_ibm_mulaw = 257;
static const std::uint16_t wav_format_ibm_alaw = 258;
static const std::uint16_t wav_format_adpcm = 259;

bool validate_wav_header(const wav_header_t& wav_header, bool strong = false)
{
	bool names_valid = std::strncmp(wav_header.chunk_id, wav_chunk_id, sizeof(wav_header.chunk_id)) == 0
		&& std::strncmp(wav_header.format_id, wav_format_id, sizeof(wav_header.format_id)) == 0
		&& std::strncmp(wav_header.sub_chunk_id, wav_sub_chunk_id, sizeof(wav_header.sub_chunk_id)) == 0
		&& (strong == false || std::strncmp(wav_header.chunk_2_id, wav_chunk_2_id, sizeof(wav_header.chunk_2_id)) == 0);

	bool sizes_valid = wav_header.sub_chunk_size == 16;

	bool params_valid = (wav_header.audio_format == wav_format_pcm)
			&& (wav_header.channels > 0)
			&& (wav_header.byte_rate == wav_header.sample_rate * wav_header.channels * wav_header.bit_per_sample / 8)
			&& (wav_header.block_align == wav_header.channels * wav_header.bit_per_sample / 8);

	return names_valid && sizes_valid && params_valid;
}

std::size_t read_wav_header(std::fstream& file, wav_header_t& wav_header)
{
	std::size_t result = 0;

	if (file.is_open())
	{
		auto tell = file.tellg();

		file.seekg(0);

		file.read(static_cast<std::ifstream::char_type*>(static_cast<void*>(&wav_header)), sizeof(wav_header));

		auto ret = file.gcount();

		if (ret == sizeof(wav_header))
		{
			if (validate_wav_header(wav_header))
			{
				std::size_t total_header_size = sizeof(wav_header);

				if (std::strncmp(wav_header.chunk_2_id, wav_chunk_2_id, sizeof(wav_header.chunk_2_id)) != 0)
				{
					total_header_size += wav_header.chunk_2_size;

					file.seekg(total_header_size);

					file.readsome(static_cast<std::ifstream::char_type*>(static_cast<void*>(&wav_header.chunk_2_id)), 8);

					ret = file.gcount();

					if (ret == 8 && std::strncmp(wav_header.chunk_2_id, file_utils::wav_chunk_2_id, sizeof(wav_header.chunk_2_id)) == 0)
					{
						result = total_header_size + 8;
					}
				}
				else
				{
					result = total_header_size;
				}
			}
		}

		file.seekg(tell);
	}

	return result;
}

bool audio_format_from_wav_header(const wav_header_t& wav_header, audio_format_t& audio_format)
{
	bool result = validate_wav_header(wav_header);

	if (result == true)
	{
		audio_format.sample_format = audio_format_t::format_from_bits(wav_header.bit_per_sample, true);
		audio_format.channels = wav_header.channels;
		audio_format.sample_rate = wav_header.sample_rate;
	}

	return result;
}

#define calc_wav_field_size(field, data_size) (sizeof(wav_header_t) - sizeof(((wav_header_t*)nullptr)->field) - offsetof(wav_header_t, field) + data_size)

bool wav_header_from_audio_format(const audio_format_t& audio_format, wav_header_t& wav_header, size_t data_size = 0)
{
	bool result = audio_format.is_valid() && audio_format.is_integer_format();

	if (result == true)
	{
		// fill name fields

		std::strncpy(wav_header.chunk_id, wav_chunk_id, sizeof(wav_header.chunk_id));
		std::strncpy(wav_header.format_id, wav_format_id, sizeof(wav_header.format_id));
		std::strncpy(wav_header.sub_chunk_id, wav_sub_chunk_id, sizeof(wav_header.sub_chunk_id));
		std::strncpy(wav_header.chunk_2_id, wav_chunk_2_id, sizeof(wav_header.chunk_2_id));

		// fill size fields

		wav_header.chunk_size = calc_wav_field_size(chunk_size, data_size);
		wav_header.sub_chunk_size = 16;
		wav_header.chunk_2_size = data_size;

		// fill format fields

		wav_header.audio_format = wav_format_pcm;
		wav_header.bit_per_sample = audio_format.bit_per_sample();
		wav_header.sample_rate = audio_format.sample_rate;
		wav_header.channels = audio_format.channels;
		wav_header.block_align = audio_format.bytes_per_sample();
		wav_header.byte_rate = audio_format.bytes_per_second();

	}

	return result;
}

} // file_utils


FileChannel::FileChannel(const audio_channel_params_t &audio_params, bool is_rotate)
	: AudioChannel(audio_params)
	, m_audio_params(audio_params)
	, m_file_name("")
	, m_total_bytes(0)
	, m_is_rotate(is_rotate)
	, m_data_pos(0)
	, m_read_size(0)
{

}

FileChannel::~FileChannel()
{
	Close();
}

bool FileChannel::Open(const std::string &device_name)
{
	bool result = false;

	Close();

	auto mode = std::ios_base::binary
			| (m_audio_params.is_playback_only()
				? std::ios_base::out
				: std::ios_base::in);

	if ( (m_audio_params.is_playback_only()
		 && m_audio_params.is_valid()
		 && m_audio_params.audio_format.is_integer_format())
		 || m_audio_params.is_recorder_only() )
	{
		m_file.open(device_name, mode);

		if (m_file.is_open())
		{
			if (IsPlayback())
			{
				result = save_header(m_audio_params.audio_format);
			}
			else
			{
				result = load_header(m_audio_params.audio_format, m_total_bytes);
			}

			if (result == false)
			{
				Close();
			}
		}

	}

	return result;
}

bool FileChannel::Close()
{
	bool result = m_file.is_open();

	if (result)
	{
		if (IsPlayback())
		{
			save_header(m_audio_params.audio_format, m_total_bytes);
		}

		m_total_bytes = 0;
		m_read_size = 0;

		m_file.close();
	}

	return result;
}

bool FileChannel::IsOpen() const
{
	return m_file.is_open();
}

const std::string& FileChannel::GetName() const
{
	return m_file_name;
}

const audio_channel_params_t& FileChannel::internal_get_audio_params() const
{
	return m_audio_params;
}

bool FileChannel::internal_set_audio_params(const audio_channel_params_t &audio_params)
{
	bool result = !m_file.is_open();

	if (result == true)
	{
		m_audio_params = audio_params;
	}

	return result;
}

std::int32_t FileChannel::internal_write(const void *data, std::size_t size, uint32_t options)
{
	m_file.write(static_cast<const char*>(data), size);

	m_total_bytes += size;

	return size;
}

std::int32_t FileChannel::internal_read(void *data, std::size_t size, uint32_t options)
{
	auto real_size = size;

	size = std::min(size, m_total_bytes - m_read_size);

	m_file.read(static_cast<char*>(data), size);

	std::int32_t result = m_file.gcount();

	m_read_size += result;

	if (m_is_rotate && m_read_size == m_total_bytes)
	{
		m_read_size = 0;
		m_file.seekg(m_data_pos);

		real_size -= size;

		std::memset(static_cast<std::uint8_t*>(data) + result, 0, real_size);

		result += real_size;
	}

	return result;
}

bool FileChannel::save_header(const audio_format_t& audio_format, std::size_t data_size)
{
	bool result = false;

	m_file.flush();

	auto tell = m_file.tellp();

	if (tell < sizeof(file_utils::wav_header_t))
	{
		tell = sizeof(file_utils::wav_header_t);
	}

	m_file.seekp(0);

	file_utils::wav_header_t wav_header;

	if (file_utils::wav_header_from_audio_format(audio_format, wav_header, data_size))
	{
		m_file.write(static_cast<const char*>(static_cast<const void*>(&wav_header)), sizeof(file_utils::wav_header_t));
		result = true;
	}

	m_file.seekp(tell);

	return result;
}

bool FileChannel::load_header(audio_format_t& audio_format, std::size_t& data_size)
{
	bool result = false;

	m_file.flush();

	auto tell = m_file.tellg();

	m_file.seekg(0);

	file_utils::wav_header_t wav_header = { 0 };

	auto ret = file_utils::read_wav_header(m_file, wav_header);

	result = ret > 0 && file_utils::audio_format_from_wav_header(wav_header, audio_format);

	if (result == true)
	{
		data_size = wav_header.chunk_2_size;

		m_data_pos = ret;

		if (tell < ret)
		{
			tell = ret;
		}
	}

	m_file.seekg(tell);

	return result;
}


} // file

} // channel

} // audio

} // media

} // common
