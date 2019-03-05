#ifndef AUDIO_PROCESSOR_H
#define AUDIO_PROCESSOR_H

#include "media/common/media_queue.h"
#include "media/common/sync_point.h"

#include "media/audio/audio_composer.h"
#include "media/audio/audio_server.h"
#include "media/audio/audio_dispatcher.h"
#include "media/audio/audio_divider.h"
#include "media/audio/audio_queue.h"
#include "media/audio/audio_mux.h"

#include "media/audio/channels/alsa/alsa_channel.h"

#include "media/audio/tools/audio_event_server.h"


namespace core
{

namespace media
{

namespace audio
{

namespace tools
{

struct audio_processor_config_t
{
	struct audio_composer_config_t
	{
		audio_format_t	audio_format;
		std::size_t		queue_size;
		std::uint32_t	jitter_ms;
	}composer_config;

	struct audio_device_config_t
	{
		channels::audio_channel_params_t channel_params;
		std::uint32_t					 duration_ms;
		std::string						 device_name;
	}recorder_config, playback_config, aux_playback_config;

	struct event_server_config_t
	{
		std::uint32_t	duration_ms;
		std::uint32_t	jittr_ms;
	}event_server_config;
};

class AudioProcessor : public SyncPoint
{
	using mutex_t = std::mutex;


	class SyncAudioReaderProxy : public IAudioReader
	{
		IAudioReader&			m_audio_reader;
		const ISyncPoint&		m_sync_point;
	public:
		SyncAudioReaderProxy(IAudioReader& audio_reader, const ISyncPoint& m_sync_point);

		// IMediaReadStatus interface
	public:
		bool CanRead() const override;

		// IAudioReader interface
	public:
		int32_t Read(const audio_format_t& audio_format, void* data, std::size_t size, uint32_t options = 0) override;
	};

	class SyncAudioWriterProxy : public IAudioWriter
	{
		IAudioWriter&			m_audio_writer;
		const ISyncPoint&		m_sync_point;
	public:
		SyncAudioWriterProxy(IAudioWriter& audio_writer, const ISyncPoint& m_sync_point);

		// IMediaWriteStatus interface
	public:
		bool CanWrite() const override;

		// IAudioWriter interface
	public:
		int32_t Write(const audio_format_t& audio_format, const void* data, std::size_t size, uint32_t options = 0) override;
	};

	audio_processor_config_t		m_config;
	mutex_t							m_mutex;

	std::atomic_bool				m_is_running;

	MediaQueue						m_composer_queue;
	AudioQueue						m_event_queue;

	channels::alsa::AlsaChannel		m_recorder_channel;
	channels::alsa::AlsaChannel		m_playback_channel;
	channels::alsa::AlsaChannel		m_aux_playback_channel;

	AudioDivider					m_audio_divider;
	AudioMux						m_audio_mux;

	AudioComposer					m_audio_composer;
	AudioServer						m_audio_server;

	IAudioStream&					m_recorder_stream;
	IAudioStream&					m_playback_stream;

	SyncAudioWriterProxy			m_recorder_stream_proxy;
	SyncAudioReaderProxy			m_playback_stream_proxy;

	AudioEventServer				m_audio_event_server;

	AudioDispatcher					m_recorder_audio_dispatcher;
	AudioDispatcher					m_playback_audio_dispatcher;

public:
	AudioProcessor(const audio_processor_config_t& config);
	~AudioProcessor() override;

	media_stream_id_t RegisterStream(const session_id_t& session_id
									 , const audio_format_t& audio_format
									 , bool is_writer);

	bool UnregisterStream(media_stream_id_t audio_stream_id);

	std::size_t Write(media_stream_id_t audio_stream_id, const void* data, std::size_t size, std::uint32_t options = 0);
	std::size_t Read(media_stream_id_t audio_stream_id, void* data, std::size_t size, std::uint32_t options = 0);

	std::size_t Write(media_stream_id_t audio_stream_id, const audio_format_t& audio_format, const void* data, std::size_t size, std::uint32_t options = 0);
	std::size_t Read(media_stream_id_t audio_stream_id, const audio_format_t& audio_format, void* data, std::size_t size, std::uint32_t options = 0);

	const IAudioStream* operator[](media_stream_id_t audio_stream_id);

	const audio_processor_config_t& GetConfig() const;

	IVolumeController& GetRecorderVolumeController();
	IVolumeController& GetPlaybackVolumeController();
	IVolumeController& GetEventsVolumeController();

private:
	bool check_and_conrtol_audio_system();
	bool control_audio_system(bool is_start);
};

} // tools

} // audio

} // media

} // core

#endif // AUDIO_PROCESSOR_H
