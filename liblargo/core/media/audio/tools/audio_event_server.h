#ifndef AUDIO_EVENT_H
#define AUDIO_EVENT_H

#include "core/media/common/i_media_point.h"
#include "core/media/common/i_data_collection.h"
#include "core/media/common/delay_timer.h"
#include "core/media/common/sync_point.h"
#include "core/media/audio/i_audio_point.h"
#include "core/media/audio/channels/file/file_channel.h"


#include <unordered_map>
#include <thread>
#include <mutex>
#include <atomic>

namespace core
{

namespace media
{

namespace audio
{

namespace tools
{

class AudioEventServer: public IDataCollection, public IVolumeController
{
	class AudioEvent : public IAudioReader
	{
		std::string						m_file_name;
		std::uint32_t					m_times;
		std::uint32_t					m_inteval;

		DelayTimer						m_delay_timer;
		channels::file::FileChannel		m_file;
		std::uint32_t					m_step;
		std::size_t						m_ref_count;

	public:
		AudioEvent(const std::string& file_name
								 , std::uint32_t times
								 , std::uint32_t interval);

		void Reset(const std::string& file_name, std::uint32_t times, std::uint32_t interval);
		void Reset();

		void Start();
		void Stop();

		bool IsPlay() const;

		std::int32_t Read(const audio_format_t& audio_format, void* data, std::size_t size, std::uint32_t options = 0) override;
		bool CanRead() const override;

	};

    using event_map_t = std::unordered_map<std::string, AudioEvent>;

	SyncPoint					m_sync_point;
	audio_format_t				m_audio_format;
	std::thread					m_event_thread;
	std::uint32_t				m_duration_ms;
	std::atomic_bool			m_running;

	VolumeController			m_volume_controller;

	event_map_t					m_events;

	std::vector<std::uint8_t>	m_audio_buffer;
	std::vector<std::uint8_t>	m_mix_buffer;

	// Dependencies
private:
	IAudioWriter&				m_audio_writer;

public:

	AudioEventServer(IAudioWriter& audio_writer, const audio_format_t& audio_format, std::uint32_t duration_ms);
	~AudioEventServer();

	// IDataCollection interface
public:
	std::size_t Count() const override;

	bool AddEvent(const std::string& event_name, const std::string& file_name, std::uint32_t times, std::uint32_t interval);
	bool RemoveEvent(const std::string& event_name);

	bool PlayEvent(const std::string& event_name);
	bool StopEvent(const std::string& event_name);
	void Stop();

private:

	void event_proc();

	// IVolumeController interface
public:
	uint32_t GetVolume() const override;
	void SetVolume(uint32_t volume) override;
	bool IsMute() const override;
	void SetMute(bool mute) override;
};

} // tools

} // audio

} // media

} // core

#endif // AUDIO_EVENT_H
