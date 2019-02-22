#ifndef AUDIO_SERVER_H
#define AUDIO_SERVER_H


#include "media/audio/i_audio_server.h"
#include "media/audio/i_audio_composer.h"

#include <unordered_map>
#include <memory>

namespace core
{

namespace media
{

namespace audio
{

class AudioServer : public IAudioServer
{
	using audio_stream_t = std::shared_ptr<IAudioStream>;
	using audio_stream_map_t = std::unordered_map<media_stream_id_t, audio_stream_t>;

	IAudioComposer&			m_audio_composer;
	audio_stream_map_t		m_streams;

public:
	AudioServer(IAudioComposer& audio_composer);
	virtual ~AudioServer() override = default;


	// IAudioServer interface
public:
	IAudioStream* operator [](media_stream_id_t stream_id) override;
	const IAudioStream* operator [](media_stream_id_t stream_id) const override;

	IAudioStream* AddStream(const audio_format_t& audio_format, const session_id_t& session_id) override;
	bool RemoveStream(media_stream_id_t stream_id) override;

	// IDataCollection interface
public:
	std::size_t Count() const override;

};

} // audio

} // media

} // core

#endif // AUDIO_SERVER_H
