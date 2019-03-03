#ifndef AUDIO_SERVER_H
#define AUDIO_SERVER_H


#include "media/audio/i_audio_server.h"
#include "media/audio/i_audio_composer.h"
#include "media/audio/i_audio_slot.h"

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

	using session_descriptor_t = std::pair<audio_slot_id_t, std::size_t>;
	using session_descriptor_map_t = std::unordered_map<session_id_t, session_descriptor_t>;

	audio_stream_map_t			m_streams;
	session_descriptor_map_t	m_sessions;

	media_stream_id_t			m_stream_id;
	audio_slot_id_t				m_slot_id;

	// Dependencies
private:
	IAudioComposer&				m_audio_composer;

public:
	AudioServer(IAudioComposer& audio_composer);
	virtual ~AudioServer() override = default;


	// IAudioServer interface
public:
	IAudioStream* operator [](media_stream_id_t stream_id) override;
	const IAudioStream* operator [](media_stream_id_t stream_id) const override;

	IAudioStream* AddStream(const audio_format_t& audio_format, const session_id_t& session_id, bool is_writer) override;
	bool RemoveStream(media_stream_id_t stream_id) override;

	// IDataCollection interface
public:
	std::size_t Count() const override;

	// IAudioFormatter interface
public:
	const audio_format_t& GetAudioFormat() const override;
	bool SetAudioFormat(const audio_format_t& audio_format) override;

private:
	media_stream_id_t get_stream_id();

	IAudioSlot* request_slot(const session_id_t& session_id);
	std::size_t release_slot(const session_id_t& session_id);

};

} // audio

} // media

} // core

#endif // AUDIO_SERVER_H
