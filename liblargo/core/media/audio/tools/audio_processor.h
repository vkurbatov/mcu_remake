#ifndef AUDIO_PROCESSOR_H
#define AUDIO_PROCESSOR_H

#include "core/media/common/i_process_state_notifier.h"
#include "core/media/common/media_queue.h"
#include "core/media/common/sync_point.h"

#include "core/media/audio/audio_composer.h"
#include "core/media/audio/audio_server.h"
#include "core/media/audio/audio_dispatcher.h"
#include "core/media/audio/audio_divider.h"
#include "core/media/audio/audio_queue.h"
#include "core/media/audio/audio_mux.h"
#include "core/media/audio/i_audio_processing.h"

#include "core/media/audio/channels/alsa/alsa_channel.h"

#include "core/media/audio/tools/audio_event_server.h"

#include <vector>


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
		std::size_t		queue_duration_ms;
		std::uint32_t	compose_window_ms;
		std::uint32_t	read_delay_ms;
		std::uint32_t	dead_zone_ms;
	} composer_config;

	struct audio_device_config_t
	{
		channels::audio_channel_params_t channel_params;
		std::uint32_t					 duration_ms;
		std::string						 device_name;
	} recorder_config, playback_config, aux_playback_config;

	struct event_server_config_t
	{
		std::uint32_t	duration_ms;
		std::uint32_t	jittr_ms;
	} event_server_config;
};

class AudioProcessor : public SyncPoint, private IProcessStateNotifier
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

	class AudioProcessingPoint : public IAudioPoint
	{
		IAudioProcessing*		m_audio_processing;
		// media_buffer_t			m_reader_buffer;
		media_buffer_t			m_writer_buffer;

		IAudioReader&			m_audio_reader;
		IAudioWriter&			m_audio_writer;
	public:
		AudioProcessingPoint(IAudioReader& audio_reader, IAudioWriter& audio_writer, IAudioProcessing* audio_processing);

		// IMediaWriteStatus interface
	public:
		bool CanWrite() const override;

		// IMediaReadStatus interface
	public:
		bool CanRead() const override;

		// IAudioWriter interface
	public:
		int32_t Write(const audio_format_t& audio_format, const void* data, std::size_t size, uint32_t options) override;

		// IAudioReader interface
	public:
		int32_t Read(const audio_format_t& audio_format, void* data, std::size_t size, uint32_t options) override;

		void Reset();
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

	AudioProcessingPoint			m_audio_processing_point;

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

	static const std::vector<std::string>& GetDeviceList(bool is_recorder, bool update = false);

public:
	explicit AudioProcessor(const audio_processor_config_t& config, IAudioProcessing* external_audio_processing = nullptr);
	AudioProcessor(const AudioProcessor&) = delete;
	AudioProcessor(AudioProcessor&&) = delete;
	AudioProcessor& operator=(const AudioProcessor&) = delete;
	AudioProcessor& operator=(AudioProcessor&&) = delete;

	~AudioProcessor() override;

	media_stream_id_t RegisterStream(const session_id_t& session_id
	                                 , const audio_format_t& audio_format
	                                 , bool is_writer);

	bool UnregisterStream(media_stream_id_t audio_stream_id);

	std::size_t Write(media_stream_id_t audio_stream_id, const void* data, std::size_t size, std::uint32_t options = 0);
	std::size_t Read(media_stream_id_t audio_stream_id, void* data, std::size_t size, std::uint32_t options = 0);

	std::size_t Write(media_stream_id_t audio_stream_id, const audio_format_t& audio_format, const void* data, std::size_t size, std::uint32_t options = 0);
	std::size_t Read(media_stream_id_t audio_stream_id, const audio_format_t& audio_format, void* data, std::size_t size, std::uint32_t options = 0);

	bool SetStreamAudioFormat(media_stream_id_t audio_stream_id, const audio_format_t& audio_format_t);

	bool ResetStream(media_stream_id_t audio_stream_id);

	const IAudioStream* operator[](media_stream_id_t audio_stream_id);

	const audio_processor_config_t& GetConfig() const;

	IVolumeController& GetRecorderVolumeController();
	IVolumeController& GetPlaybackVolumeController();
	IVolumeController& GetEventVolumeController();

	AudioEventServer& GetEventServer();

	const std::string& GetRecorderDeviceName() const;
	const std::string& GetPlaybackDeviceName() const;

	bool SetRecorderDeviceName(const std::string& device_name);
	bool SetPlaybackDeviceName(const std::string& device_name);

	void Reset();

private:
    bool check_and_conrtol_audio_system();
	bool control_audio_system(bool is_start);
    bool rename_audio_device(const std::string& device_name, bool is_recorder);

	// IProcessStateNotifier interface
public:
	void StateChangeNotify(const ProcessState &new_state, const ProcessState &old_state, void *context) override;
};

} // tools

} // audio

} // media

} // core

#endif // AUDIO_PROCESSOR_H
