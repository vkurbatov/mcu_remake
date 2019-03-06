#ifndef AUDIO_DISPATCHER_H
#define AUDIO_DISPATCHER_H

#include "media/audio/i_audio_formatter.h"
#include "media/audio/i_audio_point.h"

#include <thread>
#include <atomic>



namespace core
{

namespace media
{

namespace audio
{

class AudioDispatcher
{

	std::thread				m_dispatch_thread;
	std::atomic_bool		m_is_running;
	std::atomic_uint32_t	m_cycle_counter;
	bool					m_is_strong_sizes;

	// Dependencies
private:
	IAudioReader&			m_audio_reader;
	IAudioWriter&			m_audio_writer;
	const audio_format_t&	m_audio_format;

public:
	AudioDispatcher(IAudioReader& audio_reader,
					IAudioWriter& audio_writer,
					const audio_format_t& audio_format,
					bool is_strong_sizes = false);
	~AudioDispatcher();

	bool Start(std::uint32_t duration_ms);
	bool Stop();

	bool IsRunning() const;
	std::uint32_t GetCyclesCount() const;


private:
	void dispatching_proc(std::uint32_t duration_ms);
};

} // audio

} // media

} // core

#endif // AUDIO_DISPATCHER_H
