#include <iostream>
#include <thread>
#include <cstring>
#include <mutex>

// #ifdef NODEF

#include "core/media/audio/channels/alsa/alsa_channel.h"
#include "core/media/common/data_queue.h"
#include "core/media/audio/audio_resampler.h"

void test_alsa()
{
	int i = 0;

	auto device_playback_list = core::media::audio::channels::alsa::AlsaChannel::GetDeviceList(false);
	auto device_recorder_list = core::media::audio::channels::alsa::AlsaChannel::GetDeviceList(true);

	std::cout << "ALSA playback list " << device_playback_list.size() << ":" << std::endl;

	for (const auto& info : device_playback_list)
	{
		std::cout << "#" << i << ": " << info.native_format() << ", " << info.display_format() << std::endl;
		i++;
	}

	i = 0;

	std::cout << "ALSA recorder list " << device_recorder_list.size() << ":" << std::endl;

	for (const auto& info : device_recorder_list)
	{
		std::cout << "#" << i << ": " << info.native_format() << ", " << info.display_format() << std::endl;
		i++;
	}


	static const std::uint32_t duration_ms = 10;
	static const std::uint32_t recorder_sample_rate = 44100;
	static const std::uint32_t recorder_frame_size = 2 * (recorder_sample_rate * duration_ms) / 1000;
	static const std::uint32_t playback_sample_rate = 32000;
	static const std::uint32_t playback_frame_size = 2 * (playback_sample_rate * duration_ms) / 1000;
	static const std::uint32_t buffers_count = 10;

	using clock = std::chrono::high_resolution_clock;

	core::media::DataQueue queue(recorder_frame_size * buffers_count);
	std::mutex  queue_mutex;

	std::thread recorder_thread([&device_recorder_list, &queue_mutex, &queue]()
	{
		core::media::audio::channels::audio_channel_params_t recorder_params(core::media::audio::channels::channel_direction_t::recorder, { recorder_sample_rate, core::media::audio::audio_format_t::sample_format_t::pcm_16, 1 }, duration_ms, true);

		core::media::audio::channels::alsa::AlsaChannel recorder(recorder_params);

		recorder.Open("default");

		recorder.SetVolume(100);

		std::uint8_t buffer[recorder_frame_size];

		auto start = std::chrono::high_resolution_clock::now();

		while (recorder.IsOpen())
		{
			auto t_0 = clock::now();

			auto ret = recorder.Read(buffer, recorder_frame_size);

			{
				std::lock_guard<decltype(queue_mutex)> gl(queue_mutex);

				queue.Push(buffer, ret);
			}


			start += std::chrono::milliseconds(duration_ms);

			std::this_thread::sleep_for(start - clock::now());

			auto delay = std::chrono::duration_cast<std::chrono::microseconds>(clock::now() - t_0).count();

			// std::cout << "Read delay = "  << delay << ", ret = " << ret << std::endl;
		}
	}
	);

	std::thread player_thread([&device_playback_list, &queue_mutex, &queue]()
	{
		core::media::audio::channels::audio_channel_params_t player_params(core::media::audio::channels::channel_direction_t::playback, { 48000, core::media::audio::audio_format_t::sample_format_t::pcm_16, 2 }, duration_ms, true);

		core::media::audio::channels::alsa::AlsaChannel player(player_params);

		player.Open("default");

		player.SetVolume(100);


		std::uint8_t buffer[100000];

		std::this_thread::sleep_for(std::chrono::milliseconds(500));

		auto start = clock::now();

		core::media::audio::audio_format_t input_format(recorder_sample_rate, core::media::audio::audio_format_t::sample_format_t::pcm_16, 1);

		while (player.IsOpen())
		{

			auto t_0 = clock::now();

			std::size_t q_size = 0;
			std::size_t q_lost = 0;
			{
				std::lock_guard<decltype(queue_mutex)> gl(queue_mutex);

				q_size = queue.Pop(buffer, recorder_frame_size);
				q_lost = queue.Size();

			}

			if (q_size > 0)
			{

				player.Write(input_format, buffer, q_size );

				/*auto rs_size = core::media::audio::AudioResampler::Resampling(input_format, player_params.audio_format, buffer, q_size, resample_buffer);

				auto io_size = player.Write(resample_buffer, rs_size);

				if (io_size != playback_frame_size)
				{
					std::cout << "Playback error, q_size = "  << q_size << ":" << q_lost << ", rs_size = " << rs_size << ", io_size = " << io_size << ", frame_size = " << playback_frame_size << std::endl;
				}*/
			}

			start += std::chrono::milliseconds(duration_ms);
			//std::this_thread::sleep_for(std::chrono::milliseconds(15));

			std::this_thread::sleep_for(start - clock::now());

			auto delay = std::chrono::duration_cast<std::chrono::microseconds>(clock::now() - t_0).count();

			// std::cout << "Write delay = "  << delay << ", ret = " << ret << std::endl;
		}
	}

	);

	recorder_thread.join();
	player_thread.join();
}

#include <media/common/multipoint_data_queue.h>

void print_buffer(const void* data, std::size_t size)
{
	auto data_ptr = static_cast<const std::uint8_t*>(data);

	// std::cout << std::hex;

	for (auto i = 0ul; i < size; i++)
	{
		if (i > 0ul)
		{
			std::cout << "-";
		}

		std::cout << static_cast<std::int32_t>(*(data_ptr + i));
	}

	// std::cout <<std::dec;
}

void print_buffer_ln(const void* data, std::size_t size)
{
	print_buffer(data, size);
	std::cout << std::endl;
}

void print_queue_status(const core::media::MultipointDataQueue& queue, core::media::cursor_t cursor)
{
	std::cout << "cursor = " << cursor << ", r_cursor = " << queue.GetReadCursor() << ", w_cursor = " << queue.GetWriteCursor()
			  << ", size = " << queue.Size() << ", b_size " << queue.GetDataSize(cursor, true) << ", a_size = " << queue.GetDataSize(cursor) << std::endl;

	std::vector<std::uint8_t> read_buffer(queue.Size());

	auto result = queue.Read(queue.GetReadCursor(), read_buffer.data(), queue.GetDataSize(cursor, true));

	std::cout << "before data [" << result << "]: " << std::endl;

	print_buffer_ln(read_buffer.data(), result);

	result = queue.Read(cursor, read_buffer.data(), queue.GetDataSize(cursor));

	std::cout << "after data [" << result << "]: " << std::endl;

	print_buffer_ln(read_buffer.data(), result);


}

void test_queue()
{
	const std::size_t buffer_size = 14;

	std::uint8_t buffer_1[] { 0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32 };
	std::uint8_t buffer_2[] { 1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33 };

	std::uint8_t read_buffer[buffer_size * 2];

	core::media::MultipointDataQueue queue(buffer_size);

	std::cout << "\nTest 1. write to queue array:" << std::endl;

	print_buffer_ln(buffer_1, sizeof(buffer_1));

	auto result = queue.Write(0, buffer_1, sizeof(buffer_1));

	std::cout << "Write result = " << result << ". Queue status: " << std::endl;

	print_queue_status(queue, 12);

	//--------------------------------------------------------

	std::cout << "\nTest 2. rewrite to queue array:" << std::endl;

	print_buffer_ln(buffer_2, 4);

	result = queue.Write(5, buffer_2, 4);

	std::cout << "Rewrite result = " << result << ". Queue status: " << std::endl;

	print_queue_status(queue, 12);

	//--------------------------------------------------------

	std::cout << "\nTest 3. modify save queue data" << std::endl;

	result = queue.Read(6, read_buffer, 4);

	std::cout << "Read result = " << result << ". Read array: " << std::endl;

	print_buffer_ln(read_buffer, result);

	for (int i = 0; i < result; i++)
	{
		read_buffer[i] += 40;
	}

	result = queue.Write(6, read_buffer, result);

	std::cout << "Modify result = " << result << ". Queue status: " << std::endl;

	print_queue_status(queue, 12);

	//--------------------------------------------------------

	std::cout << "\nTest 4. modify extra queue data" << std::endl;

	result = queue.Read(13, read_buffer, 10);

	std::cout << "Read result = " << result << ". Read array: " << std::endl;

	print_buffer_ln(read_buffer, result);

	for (int i = 0; i < result + 2; i++)
	{
		read_buffer[i] = 80 + i;
	}

	result = queue.Write(13, read_buffer, result + 2);

	std::cout << "Extra modify result = " << result << ". Queue status: " << std::endl;

	print_queue_status(queue, 12);


}

#include "media/common/media_queue.h"

void test_media_queue()
{
	const auto queue_size = 14;

	const core::media::media_slot_id_t slot_id_1 = 1, slot_id_2 = 2;

	core::media::MediaQueue media_queue(queue_size);

	core::media::IMediaQueue& queue = media_queue;

	std::uint8_t w_buffer_1[] = { 1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,  16,  17,  18 };
	std::uint8_t w_buffer_2[] = { 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118 };

	std::uint8_t r_buffer_1[sizeof(w_buffer_1)] = { 0 };
	std::uint8_t r_buffer_2[sizeof(w_buffer_2)] = { 0 };

	queue.QuerySlot(slot_id_1);
	queue.QuerySlot(slot_id_2);

	auto w_res_1 = queue[slot_id_1]->Push(w_buffer_1, 8 /*sizeof(w_buffer_1)*/);
	auto w_res_2 = queue[slot_id_2]->Push(w_buffer_2, 4);

	auto r_res_1 = queue[slot_id_1]->Read(r_buffer_1, sizeof(r_buffer_1));
	auto r_res_2 = queue[slot_id_2]->Read(r_buffer_2, sizeof(r_buffer_2), true);

	auto rem_res_1 = queue.ReleaseSlot(slot_id_1);
	auto rem_res_2 = queue.ReleaseSlot(slot_id_2);

	std::cout << "/nTest 1. Media Queue. Add and Remove slots. Write and Read two slots." << std::endl;
	std::cout << "Test status: w_res_1 = " << w_res_1
							<< ", w_res_2 = " << w_res_2
							<< ", r_res_1 = " << r_res_1
							<< ", r_res_2 = " << r_res_2
							<< ", rem_res_1 = " << rem_res_1
							<< ", rem_res_2 = " << rem_res_2
							<< std::endl;

	std::cout << "Buffer 1: " << std::endl;
	print_buffer_ln(r_buffer_1, r_res_1);

	std::cout << "Buffer 2: " << std::endl;
	print_buffer_ln(r_buffer_2, r_res_2);

}

#include "media/audio/channels/file/file_channel.h"
#include "media/common/delay_timer.h"

void test_audio_file()
{
	core::media::audio::channels::audio_channel_params_t r_params;
	r_params.direction = core::media::audio::channels::channel_direction_t::recorder;

	core::media::audio::channels::file::FileChannel r_file(r_params);


	if (r_file.Open("/home/vkurbatov/ivcscodec/test_sound/Front_Center.wav"))
	{
		const auto& params = r_file.GetAudioParams();
		std::cout << "sample_rate = " << params.audio_format.sample_rate
				  << "Hz , bit_per_sample = " << params.audio_format.bit_per_sample()
				  << ", channels = " << params.audio_format.channels
				  << std::endl;
	}

	auto w_param = r_file.GetAudioParams();

	w_param.audio_format.sample_rate = 48000;
	w_param.buffer_duration_ms = 20;
	w_param.nonblock_mode = 1;

	w_param.direction = core::media::audio::channels::channel_direction_t::playback;

	core::media::audio::channels::file::FileChannel w_file(w_param);

	core::media::audio::channels::alsa::AlsaChannel w_alsa(w_param);

	w_file.Open("/home/vkurbatov/ivcscodec/test_sound/Front_Center2.wav");

	w_alsa.Open("default");

	core::media::audio::channels::IAudoChannel& w_channel = w_alsa;

	core::media::audio::IAudioPoint& w_point = w_alsa;

	core::media::DelayTimer	timer;

	if (w_channel.IsOpen())
	{
		auto part_size = r_file.GetAudioParams().audio_format.size_from_duration(w_param.buffer_duration_ms);

		char buffer[10000];

		while (auto ret = r_file.Read(buffer, part_size))
		{
			w_point.Write(r_file.GetAudioParams().audio_format, buffer, ret);
			timer(w_param.buffer_duration_ms);
		}
	}
}

#include "media/audio/channels/audio_channel_worker.h"

void test_audio_channel_worker()
{
	static const std::uint32_t duration_ms = 10;
	static const std::uint32_t recorder_sample_rate = 32000;
	static const std::uint32_t playback_sample_rate = 48000;

	core::media::audio::channels::audio_channel_params_t recorder_params(core::media::audio::channels::channel_direction_t::recorder, { recorder_sample_rate, core::media::audio::audio_format_t::sample_format_t::pcm_16, 1 }, duration_ms, true);

	core::media::audio::channels::alsa::AlsaChannel recorder(recorder_params);

	core::media::audio::channels::audio_channel_params_t player_params(core::media::audio::channels::channel_direction_t::playback, { playback_sample_rate, core::media::audio::audio_format_t::sample_format_t::pcm_16, 1 }, duration_ms, true);

	core::media::audio::channels::alsa::AlsaChannel player(player_params);

	core::media::audio::channels::AudioChannelWorker recorder_worker(recorder, recorder, 64000);
	core::media::audio::channels::AudioChannelWorker player_worker(player, player, 64000);


	recorder_worker.Open("default");

	player_worker.Open("default");

	std::uint8_t buffer[10000];

	auto part_size = recorder_params.buffer_size();

	core::media::DelayTimer	timer;

	while(true)
	{
		auto result = recorder_worker.Read(buffer, part_size);

		result = player_worker.Write(recorder_params.audio_format, buffer, result);

		timer(duration_ms);
	}
}

#include "media/audio/audio_dispatcher.h"

void test_audio_dispatcher()
{
	static const std::uint32_t duration_ms = 10;
	static const std::uint32_t recorder_sample_rate = 32000;
	static const std::uint32_t playback_sample_rate = 48000;

	core::media::audio::channels::audio_channel_params_t recorder_params(core::media::audio::channels::channel_direction_t::recorder, { recorder_sample_rate, core::media::audio::audio_format_t::sample_format_t::pcm_16, 1 }, duration_ms, true);

	core::media::audio::channels::alsa::AlsaChannel recorder(recorder_params);

	core::media::audio::channels::audio_channel_params_t player_params(core::media::audio::channels::channel_direction_t::playback, { playback_sample_rate, core::media::audio::audio_format_t::sample_format_t::pcm_16, 1 }, duration_ms, true);

	core::media::audio::channels::alsa::AlsaChannel player(player_params);

	core::media::audio::AudioDispatcher dispatcher(recorder, player, player_params.audio_format);

	core::media::DelayTimer timer;

	recorder.Open("default");
	player.Open("default");

	dispatcher.Start(duration_ms);

	while(dispatcher.IsRunning()) timer(duration_ms);
}

#include "media/audio/audio_composer.h"
#include "media/audio/audio_server.h"
#include "media/audio/audio_string_format_utils.h"

void test_composer()
{
	static const std::uint32_t duration_ms = 10;
	static const std::uint32_t recorder_sample_rate = 8000;
	static const std::uint32_t playback_sample_rate = 32000;

	static const std::uint32_t composer_sample_rate = 48000;

	static const std::uint32_t file_sample_rate = 16000;

	const std::string alsa_session_id = "local_audio";
	const std::string file_session_id1 = "file_audio1";
	const std::string file_session_id2 = "file_audio2";

	core::media::audio::audio_format_t composer_audio_format = { composer_sample_rate, core::media::audio::audio_format_t::sample_format_t::pcm_16, 1 };

	static const std::size_t media_queue_size = 64000;

	core::media::audio::channels::audio_channel_params_t recorder_params(core::media::audio::channels::channel_direction_t::recorder, { recorder_sample_rate, core::media::audio::audio_format_t::sample_format_t::pcm_16, 1 }, duration_ms, true);

	core::media::audio::channels::alsa::AlsaChannel recorder(recorder_params);

	core::media::audio::channels::audio_channel_params_t player_params(core::media::audio::channels::channel_direction_t::playback, { playback_sample_rate, core::media::audio::audio_format_t::sample_format_t::pcm_16, 1 }, duration_ms, true);

	core::media::audio::channels::alsa::AlsaChannel player(player_params);

	core::media::audio::channels::audio_channel_params_t r_file_params(core::media::audio::channels::channel_direction_t::recorder, core::media::audio::null_audio_format, duration_ms, false);
	core::media::audio::channels::audio_channel_params_t w_file_params1(core::media::audio::channels::channel_direction_t::playback, { file_sample_rate, core::media::audio::audio_format_t::sample_format_t::pcm_16, 2 }, duration_ms, true);

	core::media::audio::channels::file::FileChannel r_file1(r_file_params, 2);
	core::media::audio::channels::file::FileChannel r_file2(r_file_params, 60);
	core::media::audio::channels::file::FileChannel w_file1(w_file_params1);

	// core::media::audio::tools::AudioEvent event("/home/vkurbatov/ivcscodec/test_sound/Side_Left.wav", 3, 3000);

	core::media::MediaQueue media_queue(media_queue_size);

	core::media::audio::AudioComposer audio_composer(composer_audio_format, media_queue, 60);

	core::media::audio::AudioServer audio_server(audio_composer);

	recorder.Open("default");
	player.Open("default");
	r_file1.Open("/home/vkurbatov/ivcscodec/test_sound/Front_Center.wav");
	r_file2.Open("/home/vkurbatov/ivcscodec/test_sound/Side_Left.wav");
	w_file1.Open("/home/vkurbatov/ivcscodec/test_sound/Compose.wav");

	// core::media::audio::AudioDispatcher file_dispatcher(r_file, player, r_file.GetAudioFormat(), true);

	// file_dispatcher.Start(duration_ms * 2);

	auto* read_audio_stream = audio_server.AddStream(alsa_session_id, player.GetAudioFormat(), false);
	auto* write_audio_stream = audio_server.AddStream(alsa_session_id, recorder.GetAudioFormat(), true);
	auto* r_file_audio_stream1 = audio_server.AddStream(file_session_id1, r_file1.GetAudioFormat(), true);
	auto* r_file_audio_stream2 = audio_server.AddStream(file_session_id2, r_file2.GetAudioFormat(), true);
	auto* w_file_audio_stream1 = audio_server.AddStream(file_session_id1, w_file1.GetAudioFormat(), false);

	core::media::audio::AudioDispatcher player_dispatcher(*read_audio_stream, player, player.GetAudioFormat(), true);
	core::media::audio::AudioDispatcher recorder_dispatcher(recorder, *write_audio_stream, recorder.GetAudioFormat(), true);
	core::media::audio::AudioDispatcher r_file_dispatcher1(r_file1, *r_file_audio_stream1, r_file1.GetAudioFormat(), true);
	core::media::audio::AudioDispatcher r_file_dispatcher2(r_file2, *r_file_audio_stream2, r_file2.GetAudioFormat(), true);
	core::media::audio::AudioDispatcher w_file_dispatcher1(*w_file_audio_stream1, w_file1, w_file1.GetAudioFormat(), true);

	std::cout << w_file_params1 << std::endl;

	r_file_dispatcher1.Start(duration_ms * 3);
	r_file_dispatcher2.Start(duration_ms * 2);
	w_file_dispatcher1.Start(duration_ms);
	recorder_dispatcher.Start(duration_ms * 2);
	player_dispatcher.Start(duration_ms);

	core::media::DelayTimer timer;

	auto count = 10;

	while(count-- > 0) timer(1000);
}

#include "media/audio/tools/audio_event_server.h"
#include "media/audio/audio_mux.h"
#include "media/audio/audio_queue.h"
#include "media/audio/audio_divider.h"

void test_events()
{
	static const std::uint32_t duration_ms = 10;
	static const std::uint32_t recorder_sample_rate = 8000;
	static const std::uint32_t playback_sample_rate1 = 32000;
	static const std::uint32_t playback_sample_rate2 = 16000;

	static const std::string event_1 = "event_1";
	static const std::string event_2 = "event_2";

	auto playback_device_list = core::media::audio::channels::alsa::AlsaChannel::GetDeviceList(core::media::audio::channels::channel_direction_t::playback);
	auto recorder_device_list = core::media::audio::channels::alsa::AlsaChannel::GetDeviceList(core::media::audio::channels::channel_direction_t::recorder);


	core::media::audio::channels::audio_channel_params_t recorder_params( core::media::audio::channels::channel_direction_t::recorder, { recorder_sample_rate, core::media::audio::audio_format_t::sample_format_t::pcm_16, 1 }, duration_ms * 8, true );
	core::media::audio::channels::alsa::AlsaChannel recorder(recorder_params);

	core::media::audio::channels::audio_channel_params_t player_params1(core::media::audio::channels::channel_direction_t::playback, { playback_sample_rate1, core::media::audio::audio_format_t::sample_format_t::pcm_16, 1 }, duration_ms, true);
	core::media::audio::channels::alsa::AlsaChannel player1(player_params1);

	core::media::audio::channels::audio_channel_params_t player_params2(core::media::audio::channels::channel_direction_t::playback, { playback_sample_rate2, core::media::audio::audio_format_t::sample_format_t::pcm_16, 1 }, duration_ms * 8, true);
	core::media::audio::channels::alsa::AlsaChannel player2(player_params2);

	core::media::audio::AudioQueue audio_queue(player1.GetAudioFormat(), 1000, 60, true);
	core::media::audio::tools::AudioEventServer	event_server(audio_queue, audio_queue.GetAudioFormat(), true);

	core::media::audio::AudioDivider divider(player1, player2);

	core::media::audio::AudioMux mux(divider, audio_queue);

	// mux.GetAuxVolumeController().SetVolume(100);
	// mux.GetMainVolumeController().SetVolume(100);
	// mux.GetMainVolumeController().SetMute(true);

	core::media::audio::AudioDispatcher dispatcher(recorder, mux, player1.GetAudioFormat(), duration_ms * 2);

	player1.Open("default");
	player2.Open(playback_device_list[1].display_format());
	recorder.Open(recorder_device_list[1].display_format());

	// core::media::DelayTimer::Sleep(1000);

	event_server.AddEvent(event_1, "/home/vkurbatov/ivcscodec/test_sound/Front_Center.wav", 3, 2000);
	event_server.AddEvent(event_2, "/home/vkurbatov/ivcscodec/test_sound/Side_Left.wav", 4, 1000);

	event_server.PlayEvent(event_1);
	event_server.PlayEvent(event_2);

	dispatcher.Start(duration_ms);

	core::media::DelayTimer::Sleep(20000);

	event_server.RemoveEvent(event_1);
}

#include "media/audio/tools/audio_processor.h"

class AudioProcessorWrapper : public core::media::audio::IAudioPoint
{
	core::media::audio::tools::AudioProcessor&	m_processor;
	core::media::media_stream_id_t				m_stream_id;

public:

	AudioProcessorWrapper(core::media::audio::tools::AudioProcessor& processor
						  , core::media::media_stream_id_t stream_id)
		: m_processor(processor)
		, m_stream_id(stream_id)
	{

	}

	// IMediaWriteStatus interface
public:
	bool CanWrite() const override
	{
		return true;
	}

	// IMediaReadStatus interface
public:
	bool CanRead() const override
	{
		return true;
	}

	// IAudioWriter interface
public:
	std::int32_t Write(const core::media::audio::audio_format_t& audio_format, const void* data, std::size_t size, std::uint32_t options) override
	{
		auto ret = m_processor.Write(m_stream_id, data, size, options);

		return ret;
	}

	// IAudioReader interface
public:
	std::int32_t Read(const core::media::audio::audio_format_t& audio_format, void* data, std::size_t size, std::uint32_t options) override
	{
		auto ret = m_processor.Read(m_stream_id, data, size, options);

		return ret;
	}
};

class AudioProcessing: public core::media::audio::IAudioProcessing
{

	core::media::audio::VolumeController	m_input_volume_controller;
	core::media::audio::VolumeController	m_output_volume_controller;

public:
	AudioProcessing(std::uint32_t input_volume = 100, std::uint32_t output_volume = 100)
		: m_input_volume_controller(input_volume)
		, m_output_volume_controller(output_volume)
	{

	}

	// IAudioProcessing interface
public:
	std::size_t AudioProcessingRead(const core::media::audio::audio_format_t& audio_format, const void* input_data, std::size_t input_size, void* output_data, std::size_t output_size) override
	{
		std::size_t result = input_size;

		if (output_data != nullptr)
		{
			result = m_input_volume_controller(audio_format.sample_format, input_data, input_size, output_data, output_size);
		}

		return result;
	}

	std::size_t AudioProcessingWrite(const core::media::audio::audio_format_t& audio_format, const void* input_data, std::size_t input_size, void* output_data, std::size_t output_size) override
	{
		std::size_t result = input_size;

		if (output_data != nullptr)
		{
			result = m_output_volume_controller(audio_format.sample_format, input_data, input_size, output_data, output_size);
		}

		return result;
	}

	bool CanReadProcessing() const override
	{
		return true;
	}

	bool CanWriteProcessing() const override
	{
		return true;
	}

	void Reset() override
	{

	}
};

void test_audio_processor()
{

	const std::uint32_t duration_ms = 10;
	const std::uint32_t jitter_ms = 60;

	const core::media::audio::session_id_t session_id_1 = "session_1";

	auto recorder_device_list = core::media::audio::channels::alsa::AlsaChannel::GetDeviceList(core::media::audio::channels::channel_direction_t::recorder);
	auto playback_device_list = core::media::audio::channels::alsa::AlsaChannel::GetDeviceList(core::media::audio::channels::channel_direction_t::playback);

	auto recorder1 = recorder_device_list[0].display_format();
	auto recorder2 = recorder_device_list[1].display_format();

	auto playback1 = playback_device_list[0].display_format();
	auto playback2 = playback_device_list[1].display_format();


	std::string file_event1 = "/home/vkurbatov/ivcscodec/test_sound/Front_Center.wav";

	core::media::audio::tools::audio_processor_config_t	audio_processor_config;


	audio_processor_config.composer_config.audio_format.sample_rate = 32000;
	audio_processor_config.composer_config.audio_format.sample_format = core::media::audio::audio_format_t::sample_format_t::pcm_16;
	audio_processor_config.composer_config.audio_format.channels = 1;

	audio_processor_config.composer_config.jitter_ms = 60;
	audio_processor_config.composer_config.queue_duration_ms = 2000;

	audio_processor_config.event_server_config.jittr_ms = jitter_ms;
	audio_processor_config.event_server_config.duration_ms = 2000;

	audio_processor_config.recorder_config.channel_params.direction = core::media::audio::channels::channel_direction_t::recorder;
	audio_processor_config.recorder_config.channel_params.buffer_duration_ms = duration_ms * 2;
	audio_processor_config.recorder_config.channel_params.nonblock_mode = true;

	audio_processor_config.recorder_config.channel_params.audio_format.sample_rate = 8000;
	audio_processor_config.recorder_config.channel_params.audio_format.sample_format = core::media::audio::audio_format_t::sample_format_t::pcm_16;
	audio_processor_config.recorder_config.channel_params.audio_format.channels = 1;

	audio_processor_config.recorder_config.duration_ms = duration_ms;
	audio_processor_config.recorder_config.device_name = recorder1;


	audio_processor_config.playback_config.channel_params.direction = core::media::audio::channels::channel_direction_t::playback;
	audio_processor_config.playback_config.channel_params.buffer_duration_ms = duration_ms * 2;
	audio_processor_config.playback_config.channel_params.nonblock_mode = true;

	audio_processor_config.playback_config.channel_params.audio_format.sample_rate = 48000;
	audio_processor_config.playback_config.channel_params.audio_format.sample_format = core::media::audio::audio_format_t::sample_format_t::pcm_16;
	audio_processor_config.playback_config.channel_params.audio_format.channels = 1;

	audio_processor_config.playback_config.duration_ms = duration_ms;
	audio_processor_config.playback_config.device_name = playback1;


	audio_processor_config.aux_playback_config.channel_params.direction = core::media::audio::channels::channel_direction_t::playback;
	audio_processor_config.aux_playback_config.channel_params.buffer_duration_ms = duration_ms * 8;
	audio_processor_config.aux_playback_config.channel_params.nonblock_mode = true;

	audio_processor_config.aux_playback_config.channel_params.audio_format.sample_rate = 16000;
	audio_processor_config.aux_playback_config.channel_params.audio_format.sample_format = core::media::audio::audio_format_t::sample_format_t::pcm_16;
	audio_processor_config.aux_playback_config.channel_params.audio_format.channels = 1;

	audio_processor_config.aux_playback_config.duration_ms = duration_ms;
	audio_processor_config.aux_playback_config.device_name = ""; // playback_device_list[1].user_format();


	core::media::audio::channels::audio_channel_params_t player_params;
	core::media::audio::channels::audio_channel_params_t recorder_params;

	recorder_params.direction = core::media::audio::channels::channel_direction_t::recorder;
	recorder_params.buffer_duration_ms = duration_ms * 8;
	recorder_params.nonblock_mode = true;

	recorder_params.audio_format.sample_rate = 16000;
	recorder_params.audio_format.sample_format = core::media::audio::audio_format_t::sample_format_t::pcm_16;
	recorder_params.audio_format.channels = 1;


	player_params.direction = core::media::audio::channels::channel_direction_t::playback;
	player_params.buffer_duration_ms = duration_ms * 8;
	player_params.nonblock_mode = true;

	player_params.audio_format.sample_rate = 8000;
	player_params.audio_format.sample_format = core::media::audio::audio_format_t::sample_format_t::pcm_16;
	player_params.audio_format.channels = 1;

	core::media::audio::channels::alsa::AlsaChannel alsa_recorder(recorder_params);
	core::media::audio::channels::alsa::AlsaChannel alsa_player(player_params);

	alsa_recorder.Open(recorder2);
	alsa_player.Open(playback2);

	AudioProcessing	audio_processing(3, 100);

	core::media::audio::tools::AudioProcessor audio_processor(audio_processor_config, &audio_processing);

	auto playback_stream_id = audio_processor.RegisterStream(session_id_1, alsa_player.GetAudioFormat(), false);
	auto recorder_stream_id = audio_processor.RegisterStream(session_id_1, alsa_recorder.GetAudioFormat(), true);

	AudioProcessorWrapper	playback_wrapper(audio_processor, playback_stream_id);
	AudioProcessorWrapper	recorder_wrapper(audio_processor, recorder_stream_id);

	core::media::audio::AudioDispatcher recorder_dispatcher(alsa_recorder, recorder_wrapper, alsa_recorder.GetAudioFormat(), true);
	core::media::audio::AudioDispatcher playback_dispatcher(playback_wrapper, alsa_player, alsa_player.GetAudioFormat(), true);

	recorder_dispatcher.Start(duration_ms);
	playback_dispatcher.Start(duration_ms);

	audio_processor.GetEventServer().AddEvent("incoming_call", file_event1, 3, 1000);
	audio_processor.GetEventServer().PlayEvent("incoming_call");
/*
	audio_processor.GetRecorderVolumeController().SetVolume(10);
	audio_processor.GetPlaybackVolumeController().SetVolume(3);
	audio_processor.GetEventVolumeController().SetVolume(50);*/


	core::media::DelayTimer delay;

	while(true) delay(duration_ms);
}

void test_device_list()
{
	int i = 0;

	auto device_playback_list = core::media::audio::channels::alsa::AlsaChannel::GetDeviceList(false);
	auto device_recorder_list = core::media::audio::channels::alsa::AlsaChannel::GetDeviceList(true);

	std::cout << "ALSA playback list " << device_playback_list.size() << ":" << std::endl;

	for (const auto& info : device_playback_list)
	{
		std::cout << "#" << i << ": " << info.native_format() << ", " << info.display_format() << std::endl;
		std::cout << "\t\t" << info.card_name << "/" << info.card_number << ", " << info.device_name << "/" << info.device_number << std::endl;
		i++;
	}

	i = 0;

	std::cout << "ALSA recorder list " << device_recorder_list.size() << ":" << std::endl;

	for (const auto& info : device_recorder_list)
	{
		std::cout << "#" << i << ": " << info.native_format() << ", " << info.display_format() << std::endl;
		std::cout << "\t\t" << info.card_name << "/" << info.card_number << ", " << info.device_name << "/" << info.device_number << std::endl;
		i++;
	}
}

//  #endif

#include "codec/audio/audio_codec_options.h"
#include "common/options_helper.h"

void test_options()
{
	largo::codec::audio::AudioCodecOptions	options( 8000, 6300, 1, largo::codec::audio::sample_format_t::pcm_16 );
	largo::OptionsReader opt_r(options);
	largo::OptionsWriter opt_w(options);

	struct struct_1_t
	{
		signed n_val = 0;
		unsigned u_val = 0;
		bool f_val = false;
		double db_val = 0;
		float fl_val = 0;
	}struct_1;

	struct_1.n_val = -1;
	struct_1.u_val = 2;
	struct_1.f_val = true;
	struct_1.db_val = -1234.5678;
	struct_1.fl_val = 1234.1234;

	opt_w.SetOption<std::int32_t>("opt1", -1);
	opt_w.SetOption<std::string>("opt2", "string");
	opt_w.SetOption<double>("opt3", -12345.567f);
	opt_w.SetOption("opt4", struct_1);

	auto opt1 = opt_r.GetOption<std::uint32_t>("opt1");
	auto opt2 = opt_r.GetOption<std::string>("opt2");
	auto opt3 = opt_r.GetOption<double>("opt3");
	auto opt4 = opt_r.GetOption<struct_1_t>("opt4");
	std::cout << "opt1 = " << opt1
			  << ", opt2 = " << opt2
			  << ", opt3 = " << opt3
			  << ", opt4 = " << opt4.n_val << ":" << opt4.u_val << ":" << opt4.f_val << ":" << opt4.db_val << ":" << opt4.fl_val
			  << std::endl;

}

#include "codec/audio/libav/libav_wrapper.h"

void test_libav_wrapper()
{

	auto device_playback_list = core::media::audio::channels::alsa::AlsaChannel::GetDeviceList(false);

	static const std::uint32_t duration_ms = 30;
	static const std::uint32_t recorder_sample_rate = 8000;
	static const std::uint32_t recorder_frame_size = 2 * (recorder_sample_rate * duration_ms) / 1000;
	static const std::uint32_t playback_sample_rate = 8000;
	static const std::uint32_t playback_frame_size = 2 * (playback_sample_rate * duration_ms) / 1000;
	static const std::uint32_t buffers_count = 10;

	core::media::audio::channels::audio_channel_params_t recorder_params(core::media::audio::channels::channel_direction_t::recorder, { recorder_sample_rate, core::media::audio::audio_format_t::sample_format_t::pcm_16, 1 }, duration_ms, true);
	core::media::audio::channels::audio_channel_params_t playback_params(core::media::audio::channels::channel_direction_t::playback, { playback_sample_rate, core::media::audio::audio_format_t::sample_format_t::pcm_16, 1 }, duration_ms, true);

	core::media::audio::channels::alsa::AlsaChannel	recorder(recorder_params);
	core::media::audio::channels::alsa::AlsaChannel	playback(playback_params);

	core::media::DelayTimer	delay_timer;

	recorder.Open("default");
	playback.Open(device_playback_list[3].display_format());

	largo::codec::audio::audio_codec_id_t codec_id = largo::codec::audio::audio_codec_id_t::audio_codec_g723_1;

	largo::codec::audio::libav_codec_config_t encoder_config;
	largo::codec::audio::libav_codec_config_t decoder_config;

	encoder_config.sample_rate = recorder_sample_rate;
	encoder_config.sample_format = largo::codec::audio::sample_format_t::pcm_16;
	encoder_config.frame_size = 240;
	encoder_config.bit_rate = 6300;
	encoder_config.profile = 0;

	decoder_config = encoder_config;

	largo::codec::audio::LibavWrapper	libav_encoder(codec_id, true, encoder_config);
	largo::codec::audio::LibavWrapper	libav_decoder(codec_id, false, decoder_config);

	libav_encoder.Open();
	libav_decoder.Open();

	std::uint8_t	buffer[recorder_frame_size];
	std::uint8_t	codec_buffer[recorder_frame_size];

	while(true)
	{
		auto result = recorder.Read(buffer, recorder_frame_size);

		if (result > 0)
		{
			auto enc_result = libav_encoder.Transcode(buffer, result, codec_buffer, recorder_frame_size);

			if (enc_result > 0)
			{
				auto dec_result = libav_decoder.Transcode(codec_buffer, enc_result, buffer, recorder_frame_size);

				if (dec_result > 0)
				{
					playback.Write(buffer, result);
				}
			}

		}

		delay_timer(duration_ms);
	}
}

#include "codec/audio/libav/libav_audio_transcoder.h"

void test_libav_codec_system()
{

	auto device_playback_list = core::media::audio::channels::alsa::AlsaChannel::GetDeviceList(false);

	static const std::uint32_t duration_ms = 30;
	static const std::uint32_t recorder_sample_rate = 8000;
	static const std::uint32_t recorder_frame_size = 2 * (recorder_sample_rate * duration_ms) / 1000;
	static const std::uint32_t playback_sample_rate = 8000;
	static const std::uint32_t playback_frame_size = 2 * (playback_sample_rate * duration_ms) / 1000;
	static const std::uint32_t buffers_count = 10;

	core::media::audio::channels::audio_channel_params_t recorder_params(core::media::audio::channels::channel_direction_t::recorder, { recorder_sample_rate, core::media::audio::audio_format_t::sample_format_t::pcm_16, 1 }, duration_ms, true);
	core::media::audio::channels::audio_channel_params_t playback_params(core::media::audio::channels::channel_direction_t::playback, { playback_sample_rate, core::media::audio::audio_format_t::sample_format_t::pcm_16, 1 }, duration_ms, true);

	core::media::audio::channels::alsa::AlsaChannel	recorder(recorder_params);
	core::media::audio::channels::alsa::AlsaChannel	playback(playback_params);

	core::media::DelayTimer	delay_timer;

	recorder.Open("default");
	playback.Open(device_playback_list[3].display_format());

	largo::codec::audio::audio_codec_options_t encoder_config;
	largo::codec::audio::audio_codec_options_t decoder_config;

	encoder_config.sample_rate = recorder_sample_rate;
	encoder_config.bit_rate = 6300;
	encoder_config.num_channels = 1;
	encoder_config.format = largo::codec::audio::sample_format_t::pcm_16;

	std::uint32_t libav_frame_size = 240;
	std::uint32_t libav_profile = 0;

	decoder_config = encoder_config;

	largo::codec::audio::AudioCodecOptions encoder_options(encoder_config);
	largo::codec::audio::AudioCodecOptions decoder_options(decoder_config);

	// encoder_options.SetOption(largo::codec::audio::LibavAudioTranscoder::libav_audio_codec_option_frame_size, &libav_frame_size, sizeof(libav_frame_size));
	// encoder_options.SetOption(largo::codec::audio::LibavAudioTranscoder::libav_audio_codec_option_profile, &libav_profile, sizeof(libav_profile));

	// encoder_options.SetOption(largo::codec::audio::LibavAudioTranscoder::libav_audio_codec_option_frame_size, &libav_frame_size, sizeof(libav_frame_size));
	// encoder_options.SetOption(largo::codec::audio::LibavAudioTranscoder::libav_audio_codec_option_profile, &libav_profile, sizeof(libav_profile));

	// encoder_options.SetOption("codec.audio.libav.");

	largo::codec::audio::audio_codec_id_t codec_id = largo::codec::audio::audio_codec_id_t::audio_codec_g723_1;

	largo::codec::audio::LibavAudioTranscoder libav_encoder(codec_id, true, encoder_options);
	largo::codec::audio::LibavAudioTranscoder libav_decoder(codec_id, false, decoder_options);


	libav_encoder.SetOptions(encoder_options);
	libav_decoder.SetOptions(decoder_options);

	libav_encoder.Open();
	libav_decoder.Open();

	decoder_config = encoder_config;

	std::uint8_t	buffer[recorder_frame_size];
	std::uint8_t	codec_buffer[recorder_frame_size];

	while(true)
	{
		auto result = recorder.Read(buffer, recorder_frame_size);

		if (result > 0)
		{
			auto enc_result = libav_encoder(buffer, result, codec_buffer, recorder_frame_size);

			if (enc_result > 0)
			{
				auto dec_result = libav_decoder(codec_buffer, enc_result, buffer, recorder_frame_size);

				if (dec_result > 0)
				{
					playback.Write(buffer, result);
				}
			}

		}

		delay_timer(duration_ms);
	}
}

#include <codec/audio/aac/aac_audio_transcoder.h>

void test_aac_codec()
{

	auto device_playback_list = core::media::audio::channels::alsa::AlsaChannel::GetDeviceList(false);

	static const std::uint32_t duration_ms = 10;
	static const std::uint32_t recorder_sample_rate = 48000;
	static const std::uint32_t recorder_frame_size = 2 * (recorder_sample_rate * duration_ms) / 1000;
	static const std::uint32_t playback_sample_rate = 48000;
	static const std::uint32_t playback_frame_size = 2 * (playback_sample_rate * duration_ms) / 1000;
	static const std::uint32_t buffers_count = 10;

	core::media::audio::channels::audio_channel_params_t recorder_params(core::media::audio::channels::channel_direction_t::recorder, { recorder_sample_rate, core::media::audio::audio_format_t::sample_format_t::pcm_16, 1 }, duration_ms, true);
	core::media::audio::channels::audio_channel_params_t playback_params(core::media::audio::channels::channel_direction_t::playback, { playback_sample_rate, core::media::audio::audio_format_t::sample_format_t::pcm_16, 1 }, duration_ms, true);

	core::media::audio::channels::alsa::AlsaChannel	recorder(recorder_params);
	core::media::audio::channels::alsa::AlsaChannel	playback(playback_params);

	core::media::DelayTimer	delay_timer;

	recorder.Open("default");
	playback.Open(device_playback_list[0].display_format());

	largo::codec::audio::aac_profile_id_t profile = largo::codec::audio::aac_profile_id_t::aac_profile_eld;

	largo::codec::audio::AacAudioTranscoder	aac_encoder(true, profile, recorder_sample_rate);
	largo::codec::audio::AacAudioTranscoder	aac_decoder(false, profile, recorder_sample_rate);



	aac_encoder.Open();
	aac_decoder.Open();

	std::uint8_t	buffer[recorder_frame_size];
	std::uint8_t	codec_buffer[recorder_frame_size * 4];
	std::uint8_t	resample_buffer[recorder_frame_size * 4];

	core::media::audio::audio_format_t resample_format(recorder_sample_rate, core::media::audio::audio_format_t::sample_format_t::float_32, 1);

	while(true)
	{
		auto result = recorder.Read(buffer, recorder_frame_size);

		if (result > 0)
		{

			auto resample_result = core::media::audio::AudioResampler::Resampling(recorder_params.audio_format, resample_format, buffer, result, resample_buffer, sizeof(resample_buffer));

			if (resample_result > 0)
			{
				auto enc_result = aac_encoder(resample_buffer, resample_result, codec_buffer, sizeof(codec_buffer));

				if (enc_result > 0)
				{
					auto dec_result = aac_decoder(codec_buffer, enc_result, resample_buffer, sizeof(resample_buffer));

					if (dec_result > 0)
					{
						resample_result = core::media::audio::AudioResampler::Resampling(resample_format, recorder_params.audio_format, resample_buffer, dec_result, buffer, sizeof(buffer));

						if (resample_result > 0)
						{
							playback.Write(buffer, resample_result);
						}
					}
				}

			}

		}

		delay_timer(duration_ms);
	}
}

#include "common/bit_stream.tpp"

void test_bit_stream()
{
	std::uint8_t buffer[100] = {};

	largo::BitStreamReader	reader(buffer);
	largo::BitStreamWriter	writer(buffer);

	std::size_t w_num1 = 6;
	std::size_t w_num2 = 2;
	std::size_t w_num3 = 4;

	writer.Write(&w_num1, 13);
	writer.Write(&w_num2, 3);
	writer.Write(&w_num3, 3);

	std::size_t r_num1 = 0;
	std::size_t r_num2 = 0;
	std::size_t r_num3 = 0;

	reader.Read(&r_num1, 13);
	reader.Read(&r_num2, 3);
	reader.Read(&r_num3, 3);

	return;
}

#include "codec/audio/aac/au_packetizer.h"

void test_au_packetizer()
{
	largo::codec::audio::AuPacketizer au_packetizer;
	largo::codec::audio::AuPacketizer au_depacketizer;

	char buffer[100] = {};

	au_packetizer.PushFrame("ABCD", 5);
	auto packet_size = au_packetizer.PopPacket(buffer, sizeof(buffer));

	if (packet_size > 0)
	{
		print_buffer(buffer, packet_size);
		auto frame_count = au_depacketizer.PushPacket(buffer, packet_size);

		if (frame_count > 0)
		{
			char frame[5] = {};

			au_depacketizer.PopFrame(frame, 5);

			std::cout << frame << std::endl;
		}
	}

}

int main()
{
	// test_queue();

	// test_alsa();

	// test_media_queue();

	// test_audio_file();

	// test_audio_channel_worker();

	// test_audio_dispatcher();

	// test_composer();

	// test_events();

	// test_audio_processor();

	// test_device_list();

	// test_options();

	// test_libav_wrapper();

	// test_libav_codec_system();

	// test_aac_codec();

	// test_bit_stream();

	test_au_packetizer();

	return 0;
}
