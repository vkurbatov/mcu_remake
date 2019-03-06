#ifndef I_AUDIO_STREAM_H
#define I_AUDIO_STREAM_H

#include "media/common/i_media_stream.h"
#include "media/audio/i_audio_point.h"
#include "media/audio/i_audio_formatter.h"

#include <string>

namespace core
{

namespace media
{

namespace audio
{

using session_id_t = std::string;

class IAudioStream : virtual public IMediaStream, virtual public IAudioPoint, public IAudioFormatter
{

protected:
	virtual ~IAudioStream() = default;

public:
	virtual const session_id_t& GetSessionId() const = 0;

	// IMediaWriteStatus interface
public:
	virtual bool CanWrite() const override = 0;

	// IMediaReadStatus interface
public:
	virtual bool CanRead() const override = 0;

	// IMediaWriter interface
public:
	virtual int32_t Write(const void* data, std::size_t size, uint32_t options = 0) override = 0;

	// IMediaReader interface
public:
	virtual  int32_t Read(void* data, std::size_t size, uint32_t options = 0) override = 0;
	// IAudioWriter interface
public:
	virtual int32_t Write(const audio_format_t& audio_format, const void* data, std::size_t size, uint32_t options = 0) override = 0;

	// IAudioReader interface
public:
	virtual int32_t Read(const audio_format_t& audio_format, void* data, std::size_t size, uint32_t options = 0) override = 0;
};

} // audio

} // media

} // core

#endif // I_AUDIO_STREAM_H
