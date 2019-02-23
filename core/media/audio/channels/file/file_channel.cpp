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
	std::int8_t			chunk_id[4];			// RIFF Header Magic header
	std::uint32_t       chunk_size;				// RIFF Chunk Size
	std::int8_t         format[4];				// WAVE Header
	std::int8_t         sub_chuck_id[4];		// FMT header
	std::uint32_t       sub_chuck_size;			// Size of the fmt chunk
	std::uint16_t       audio_format;			// Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
	std::uint16_t       channels;				// Number of channels 1=Mono 2=Sterio
	std::uint32_t       samples_per_seconds;	// Sampling Frequency in Hz
	std::uint32_t       byte_per_seconds;		// bytes per second
	std::uint16_t       block_align;			// 2=16-bit mono, 4=16-bit stereo
	std::uint16_t       bit_fer_sample;			// Number of bits per sample
	std::int8_t         chuck_2_id[4];			// "data"  string
	std::uint32_t       chuck_2_size;			// Sampled data length
};

static const std::int8_t wav_chunk_id = "RIFF";
static const std::int8_t wav_format = "WAVE";
static const std::int8_t wav_sub_chunk_id = "fmt ";
static const std::int8_t wav_chunk_2_id = "data";

#pragma pack(pop)

bool validate_wav_header(const wav_header_t& wav_header)
{
	bool names_valid = std::strncmp(wav_header.chunk_id, wav_chunk_id, sizeof(wav_header.chunk_id))
		&& std::strncmp(wav_header.format, wav_format, sizeof(wav_header.format))
		&& std::strncmp(wav_header.sub_chuck_id, wav_sub_chunk_id, sizeof(wav_header.sub_chuck_id))
		&& std::strncmp(wav_header.chuck_2_id, wav_chunk_2_id, sizeof(wav_header.chuck_2_id));

	bool sizes_valid =
}

} // file_utils


FileChannel::FileChannel(const audio_channel_params_t &audio_params)
	: m_audio_params(audio_params)
	, m_file_name("")
{

}

bool FileChannel::Open(const std::string &device_name)
{
	bool result = false;

	Close();

	auto mode = std::ios_base::binary
			| m_audio_params.is_playback_only()
				? std::ios_base::out
				: std::ios_base::in;

	if ( m_audio_params.is_playback_only() && m_audio_params.is_valid() || m_audio_params.is_recorder_only() )
	{
		m_file.open(device_name, mode);

		if (m_file.is_open())
		{

		}

	}

	return result;
}

bool FileChannel::Close()
{
	bool result = m_file.is_open();

	if (result)
	{
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

}

std::int32_t FileChannel::internal_read(void *data, std::size_t size, uint32_t options)
{

}


} // file

} // channel

} // audio

} // media

} // common
