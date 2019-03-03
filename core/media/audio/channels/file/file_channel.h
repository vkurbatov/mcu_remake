#ifndef FILE_CHANNEL_H
#define FILE_CHANNEL_H

#include <vector>
#include <fstream>

#include "media/audio/channels/audio_channel.h"

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

class FileChannel : public AudioChannel
{
	audio_channel_params_t	m_audio_params;
	std::string				m_file_name;
	std::fstream			m_file;

	std::size_t				m_total_bytes;

	// only recorder mode
	std::size_t				m_read_size;

	std::uint32_t			m_repetitions;
	std::uint32_t			m_current_repetition;

	std::size_t				m_data_pos;

public:
	FileChannel(const audio_channel_params_t& audio_params = null_audio_params,
				std::uint32_t repetitions = 0);
	~FileChannel() override;

	// MediaPoint interface
protected:
	int32_t internal_write(const void *data, std::size_t size, uint32_t options) override;
	int32_t internal_read(void *data, std::size_t size, uint32_t options) override;

	// IAudoChannel interface
public:
	bool Open(const std::string &device_name) override;
	bool Close() override;
	bool IsOpen() const override;
	const std::string &GetName() const override;

	// AudioChannel interface
protected:
	const audio_channel_params_t &internal_get_audio_params() const override;
	bool internal_set_audio_params(const audio_channel_params_t &audio_params) override;

private:
	bool save_header(const audio_format_t& audio_format, std::size_t data_size = 0);
	bool load_header(audio_format_t& audio_format, std::size_t& data_size);

	// IMediaReader interface
public:
	bool CanRead() const override;
	bool CanWrite() const override;
};

} // file

} // channel

} // audio

} // media

} // common

#endif // FILE_CHANNEL_H
