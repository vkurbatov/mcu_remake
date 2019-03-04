#include "audio_processor.h"

namespace core
{

namespace media
{

namespace audio
{

namespace tools
{

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
	, m_read_audio_dispatcher(*m_audio_server.AddStream(m_playback_channel.GetAudioFormat(), "local_audio", true)
							  , m_audio_mux
							  , m_playback_channel.GetAudioFormat()
							  , true)
{

}

} // tools

} // audio

} // media

} // core
