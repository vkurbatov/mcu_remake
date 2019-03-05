#include "audio_processor.h"

#include "media/common/guard_lock.h"

namespace core
{

namespace media
{

namespace audio
{

namespace tools
{

const std::string local_session_id = "local_audio";

AudioProcessor::AudioProcessor(const audio_processor_config_t& config)
	: m_config(config)
	, m_composer_queue(config.composer_config.queue_size)
	, m_event_queue(config.playback_config.channel_params.audio_format
					, config.event_server_config.duration_ms
					, config.event_server_config.jittr_ms)
	, m_recorder_channel(config.recorder_config.channel_params)
	, m_playback_channel(config.playback_config.channel_params)
	, m_aux_playback_channel(config.aux_playback_config.channel_params)
	, m_audio_divider(m_playback_channel, m_aux_playback_channel)
	, m_audio_mux(m_audio_divider, m_event_queue)
	, m_audio_composer(config.composer_config.audio_format
						, m_composer_queue
						, config.composer_config.jitter_ms)
	, m_audio_event_server(m_event_queue
							, config.playback_config.channel_params.audio_format
							, config.playback_config.channel_params.duration)
	, m_audio_server(m_audio_composer)

	, m_recorder_stream(*m_audio_server.AddStream(m_recorder_channel.GetAudioFormat(), local_session_id, true))
	, m_playback_stream(*m_audio_server.AddStream(m_playback_channel.GetAudioFormat(), local_session_id, false))

	, m_recorder_stream_proxy(m_recorder_stream, *this)
	, m_playback_stream_proxy(m_playback_stream, *this)

	, m_recorder_audio_dispatcher(m_playback_stream_proxy
								, m_audio_mux
								, m_playback_channel.GetAudioFormat()
								, true)
	, m_playback_audio_dispatcher(m_recorder_channel
								, m_recorder_stream_proxy
								, m_playback_channel.GetAudioFormat()
								, true)
{

}

AudioProcessor::SyncAudioPointProxy::SyncAudioPointProxy(IAudioPoint& audio_point
											   , const ISyncPoint& sync_point)
	: m_audio_point(audio_point)
	, m_sync_point(sync_point)
{

}

int32_t AudioProcessor::SyncAudioPointProxy::Write(const audio_format_t& audio_format, const void* data, std::size_t size, uint32_t options)
{
	GuardLock lock(m_sync_point);
	return m_audio_point.Write(audio_format, data, size, options);
}

int32_t AudioProcessor::SyncAudioPointProxy::Read(const audio_format_t& audio_format, void* data, std::size_t size, uint32_t options)
{
	GuardLock lock(m_sync_point);
	return m_audio_point.Read(audio_format, data, size, options);
}


} // tools

} // audio

} // media

} // core
