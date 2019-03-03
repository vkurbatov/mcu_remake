#ifndef AUDIO_EVENT_H
#define AUDIO_EVENT_H

#include "media/common/i_media_point.h"
#include "media/common/i_data_collection.h"
#include "media/common/delay_timer.h"
#include "media/audio/i_audio_point.h"
#include "media/audio/channels/file/file_channel.h"


#include <map>
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

class AudioEventServer: public IDataCollection
{
	class AudioEvent : public IAudioReader, public IMediaReadStatus
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

	using mutex_t = std::mutex;
	using lock_t = std::lock_guard<mutex_t>;
	using event_map_t = std::map<std::string, AudioEvent>;

	mutex_t						m_mutex;
	audio_format_t				m_audio_format;
	std::thread					m_event_thread;
	std::uint32_t				m_duration_ms;
	std::atomic_bool			m_running;

	event_map_t					m_events;

	std::vector<std::uint8_t>	m_audio_buffer;
	std::vector<std::uint8_t>	m_mix_buffer;

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

private:

	void event_proc();
};

} // tools

} // audio

} // media

} // core

#endif // AUDIO_EVENT_H
