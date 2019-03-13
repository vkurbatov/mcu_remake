#include "audio_processor.h"

#include "core/media/common/guard_lock.h"

#include <core-tools/logging.h>
#include "core/media/audio/audio_string_format_utils.h"

#define PTraceModule() "audio_event_server"

namespace core
{

namespace media
{

namespace audio
{

namespace tools
{

const session_id_t local_session_id = "local_audio";

const std::vector<std::string>& AudioProcessor::GetDeviceList(bool is_recorder, bool update)
{

	static std::vector<std::string> recorder_list;
	static std::vector<std::string> playback_list;

	auto& result = is_recorder ? recorder_list : playback_list;

	update |= result.empty();

	if (update)
	{
		const auto& device_list = audio::channels::alsa::AlsaChannel::GetDeviceList(is_recorder, update);

		result.clear();

		for (const auto& it : device_list)
		{
			result.push_back(it.display_format());
		}
	}

	return result;
}

AudioProcessor::AudioProcessor(const audio_processor_config_t& config
							  , IAudioProcessing* external_audio_processing)
	: SyncPoint(false)
	, m_config(config)
	, m_is_running(false)
	, m_composer_queue(config.composer_config.audio_format.size_from_duration(config.composer_config.queue_duration_ms), false)
	, m_event_queue(config.playback_config.channel_params.audio_format
					, config.event_server_config.duration_ms
					, config.event_server_config.jittr_ms)
	, m_recorder_channel(config.recorder_config.channel_params)
	, m_playback_channel(config.playback_config.channel_params)
	, m_aux_playback_channel(config.aux_playback_config.channel_params)
	, m_audio_divider(m_playback_channel, m_aux_playback_channel)
	, m_audio_mux(m_audio_divider, m_event_queue)
	, m_audio_processing_point(m_recorder_channel
							   , m_audio_mux
							   , external_audio_processing)

	, m_audio_composer(config.composer_config.audio_format
						, m_composer_queue
						, config.composer_config.min_jitter_ms
						, false)

	, m_audio_event_server(m_event_queue
							, config.playback_config.channel_params.audio_format
							, config.playback_config.duration_ms)
	, m_audio_server(m_audio_composer)

	, m_recorder_stream(*m_audio_server.AddStream(local_session_id, m_recorder_channel.GetAudioFormat(), true))
	, m_playback_stream(*m_audio_server.AddStream(local_session_id, m_playback_channel.GetAudioFormat(), false))

	, m_recorder_stream_proxy(m_recorder_stream, *this)
	, m_playback_stream_proxy(m_playback_stream, *this)

	, m_playback_audio_dispatcher(m_playback_stream_proxy
								, m_audio_processing_point
								, m_playback_channel.GetAudioFormat()
								, true)
	, m_recorder_audio_dispatcher(m_audio_processing_point
								, m_recorder_stream_proxy
								, m_recorder_channel.GetAudioFormat()
								, true)
{
	control_audio_system(true);
}

AudioProcessor::~AudioProcessor()
{
	control_audio_system(false);
}

media_stream_id_t AudioProcessor::RegisterStream(const session_id_t& session_id, const audio_format_t& audio_format, bool is_writer)
{
	media_stream_id_t result = media_stream_id_none;

	if (session_id != local_session_id)
	{
		{
			GuardLock lock(*this);

			auto stream = m_audio_server.AddStream(session_id, audio_format, is_writer);

			result = stream != nullptr ? stream->GetStreamId() : media_stream_id_none;
		}
	}

	return result;
}

bool AudioProcessor::UnregisterStream(media_stream_id_t audio_stream_id)
{
	bool result = false;

	if (audio_stream_id != m_recorder_stream.GetStreamId() && audio_stream_id != m_playback_stream.GetStreamId())

	{
		GuardLock lock(*this);

		result = m_audio_server.RemoveStream(audio_stream_id);
	}

	return result;

}

std::size_t AudioProcessor::Write(media_stream_id_t audio_stream_id, const void* data, std::size_t size, std::uint32_t options)
{
	GuardLock lock(*this);

	auto stream = m_audio_server[audio_stream_id];

	auto result =  stream != nullptr ? stream->Write(data, size, options) : 0;

	// LOG(debug) << "Write " << result << " bytes of " <<  size << ", for audio stream #" << audio_stream_id << ":" << stream LOG_END;

	return result;

}

std::size_t AudioProcessor::Read(media_stream_id_t audio_stream_id, void* data, std::size_t size, std::uint32_t options)
{
	GuardLock lock(*this);

	auto stream = m_audio_server[audio_stream_id];

	auto result = stream != nullptr ? stream->Read(data, size) : 0;

	// LOG(debug) << "Read " << result << " bytes of " <<  size << ", for audio stream #" << audio_stream_id << ":" << stream LOG_END;

	return result;
}

std::size_t AudioProcessor::Write(media_stream_id_t audio_stream_id, const audio_format_t& audio_format, const void* data, std::size_t size, uint32_t options)
{
	GuardLock lock(*this);

	auto stream = m_audio_server[audio_stream_id];

	return stream != nullptr ? stream->Write(audio_format, data, size, options) : 0;
}

std::size_t AudioProcessor::Read(media_stream_id_t audio_stream_id, const audio_format_t& audio_format, void* data, std::size_t size, uint32_t options)
{
	GuardLock lock(*this);

	auto stream = m_audio_server[audio_stream_id];

	return stream != nullptr ? stream->Read(audio_format, data, size) : 0;
}

bool AudioProcessor::SetStreamAudioFormat(media_stream_id_t audio_stream_id, const audio_format_t &audio_format_t)
{
	GuardLock lock(*this);

	auto stream = m_audio_server[audio_stream_id];

	return stream != nullptr && stream->SetAudioFormat(audio_format_t);
}

bool AudioProcessor::ResetStream(media_stream_id_t audio_stream_id)
{

	bool result = false;

	GuardLock lock(*this);

	auto audio_slot = m_audio_server.GetAudioSlot(audio_stream_id);

	if (audio_slot != nullptr)
	{
		audio_slot->Reset();
		result = true;
	}

	return result;
}

const IAudioStream* AudioProcessor::operator[](media_stream_id_t audio_stream_id)
{
	return m_audio_server[audio_stream_id];
}

const audio_processor_config_t& AudioProcessor::GetConfig() const
{
	return m_config;
}

IVolumeController& AudioProcessor::GetRecorderVolumeController()
{
	return m_recorder_channel;
}

IVolumeController& AudioProcessor::GetPlaybackVolumeController()
{
	return m_audio_mux.GetMainVolumeController();
}

IVolumeController& AudioProcessor::GetEventVolumeController()
{
	return m_audio_mux.GetAuxVolumeController();
}

AudioEventServer& AudioProcessor::GetEventServer()
{
	return m_audio_event_server;
}

const std::string& AudioProcessor::GetRecorderDeviceName() const
{
	return m_config.recorder_config.device_name;
}

const std::string& AudioProcessor::GetPlaybackDeviceName() const
{
	return m_config.playback_config.device_name;
}

bool AudioProcessor::SetRecorderDeviceName(const std::string &device_name)
{
	LOG(info) << "Set recorder device name \'" << device_name << "\'" LOG_END;

	if (m_config.recorder_config.device_name != device_name)
	{
		m_recorder_audio_dispatcher.Stop();

		GuardLock lock(*this);

		m_recorder_channel.Close();

		m_config.recorder_config.device_name = device_name;

		control_audio_system(true);
	}

	return true;
}

bool AudioProcessor::SetPlaybackDeviceName(const std::string &device_name)
{
	LOG(info) << "Set playback device name \'" << device_name << "\'" LOG_END;

	if (m_config.playback_config.device_name != device_name)
	{

		m_playback_audio_dispatcher.Stop();

		GuardLock lock(*this);

		m_playback_channel.Close();

		m_config.playback_config.device_name = device_name;

		control_audio_system(true);
	}

	return true;
}

bool AudioProcessor::check_and_conrtol_audio_system()
{
	auto slot_count = m_audio_composer.Count();
	// auto stream_count = m_audio_server.Count(); // ??

	bool need_running = slot_count > 1;

	if (m_is_running != need_running)
	{
		LOG(info) << "Current audio slots " << slot_count << ". Need " << (need_running ? "started" : " stopped") << " audio processor" LOG_END;
		control_audio_system(need_running);
	}

	return m_is_running;
}

bool AudioProcessor::control_audio_system(bool is_start)
{
	if (is_start)
	{
		if (!m_recorder_audio_dispatcher.IsRunning())
		{
			if (!m_config.recorder_config.device_name.empty())
			{
				m_recorder_channel.Open(m_config.recorder_config.device_name);
			}

			if (m_recorder_channel.IsOpen())
			{
				LOG(info) << "Running recorder dispatcher" LOG_END;
				m_recorder_audio_dispatcher.Start(m_config.recorder_config.duration_ms);
			}
		}

		if (!m_playback_audio_dispatcher.IsRunning())
		{
			if (!m_config.playback_config.device_name.empty())
			{
				m_playback_channel.Open(m_config.playback_config.device_name);
			}

			if (!m_config.aux_playback_config.device_name.empty())
			{
				m_aux_playback_channel.Open(m_config.aux_playback_config.device_name);
			}

			if (m_playback_channel.IsOpen() || m_aux_playback_channel.IsOpen())
			{
				LOG(info) << "Running playback dispatcher" LOG_END;
				m_playback_audio_dispatcher.Start(m_config.playback_config.duration_ms);
			}
		}
	}
	else
	{
		m_audio_event_server.Stop();
		m_recorder_audio_dispatcher.Stop();
		m_playback_audio_dispatcher.Stop();

		m_recorder_channel.Close();
		m_playback_channel.Close();
		m_aux_playback_channel.Close();

		m_event_queue.Reset();
		m_composer_queue.Reset();

		LOG(info) << "Audio processor stopped" LOG_END;
	}

	m_is_running = is_start;

	return m_is_running;
}

//--------------------------------------------------------------------------------------------------------

AudioProcessor::SyncAudioReaderProxy::SyncAudioReaderProxy(IAudioReader& audio_reader, const ISyncPoint& m_sync_point)
	: m_audio_reader(audio_reader)
	, m_sync_point(m_sync_point)
{

}

bool AudioProcessor::SyncAudioReaderProxy::CanRead() const
{
	return m_audio_reader.CanRead();
}

int32_t AudioProcessor::SyncAudioReaderProxy::Read(const audio_format_t& audio_format, void* data, std::size_t size, uint32_t options)
{
	GuardLock lock(m_sync_point);
	return m_audio_reader.Read(audio_format, data, size, options);
}

//-------------------------------------------------------------------------------------------------------

AudioProcessor::SyncAudioWriterProxy::SyncAudioWriterProxy(IAudioWriter& audio_writer, const ISyncPoint& m_sync_point)
	: m_audio_writer(audio_writer)
	, m_sync_point(m_sync_point)
{

}

bool AudioProcessor::SyncAudioWriterProxy::CanWrite() const
{
	return m_audio_writer.CanWrite();
}

int32_t AudioProcessor::SyncAudioWriterProxy::Write(const audio_format_t& audio_format, const void* data, std::size_t size, uint32_t options)
{
	GuardLock lock(m_sync_point);
	return m_audio_writer.Write(audio_format, data, size, options);
}

AudioProcessor::AudioProcessingPoint::AudioProcessingPoint(IAudioReader& audio_reader
														   , IAudioWriter& audio_writer
														   , IAudioProcessing* audio_processing)
	: m_audio_reader(audio_reader)
	, m_audio_writer(audio_writer)
	, m_audio_processing(audio_processing)
{

}

bool AudioProcessor::AudioProcessingPoint::CanWrite() const
{
	return m_audio_reader.CanRead();
}

bool AudioProcessor::AudioProcessingPoint::CanRead() const
{
	return m_audio_writer.CanWrite();
}

std::int32_t AudioProcessor::AudioProcessingPoint::Write(const audio_format_t& audio_format, const void* data, std::size_t size, std::uint32_t options)
{
	std::int32_t result = 0;

	if (m_audio_processing != nullptr)
	{

		if (m_audio_processing->CanWriteProcessing())
		{
			if (size > m_writer_buffer.size())
			{
				m_writer_buffer.resize(size);
			}

			size = m_audio_processing->AudioProcessingWrite(audio_format, data, size,  m_writer_buffer.data(), size);
			data = m_writer_buffer.data();
		}
		else
		{
			size = m_audio_processing->AudioProcessingWrite(audio_format, data, size, nullptr, 0);
		}
	}

	result = m_audio_writer.Write(audio_format, data, size, options);

	return result;
}

std::int32_t AudioProcessor::AudioProcessingPoint::Read(const audio_format_t& audio_format, void* data, std::size_t size, std::uint32_t options)
{
	std::uint32_t result = m_audio_reader.Read(audio_format, data, size, options);

	if (m_audio_processing != nullptr)
	{
		void*			output_data = m_audio_processing->CanReadProcessing() ? data : nullptr;
		std::size_t		output_size = (output_data != nullptr) ? size : 0;

		result = m_audio_processing->AudioProcessingRead(audio_format, data, size, output_data, output_size);
	}

	return result;
}

//-------------------------------------------------------------------------------------------------------

} // tools

} // audio

} // media

} // core
