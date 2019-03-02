#include <iostream>
#include <thread>
#include <cstring>
#include <mutex>

#include "core/media/audio/channels/alsa/alsa_channel.h"
#include "core/media/common/data_queue.h"
#include "core/media/audio/audio_resampler.h"

void test_alsa()
{
	int i = 0;

	auto device_playback_list = core::media::audio::channels::alsa::AlsaChannel::GetDeviceList(core::media::audio::channels::channel_direction_t::playback, "plughw");
	auto device_recorder_list = core::media::audio::channels::alsa::AlsaChannel::GetDeviceList(core::media::audio::channels::channel_direction_t::recorder, "plughw");

	std::cout << "ALSA playback list " << device_playback_list.size() << ":" << std::endl;

	for (const auto& info : device_playback_list)
	{
		std::cout << "#" << i << ": " << info.name << ", " << info.card_name << ", " << info.device_name << std::endl;
		i++;
	}

	i = 0;

	std::cout << "ALSA recorder list " << device_recorder_list.size() << ":" << std::endl;

	for (const auto& info : device_recorder_list)
	{
		std::cout << "#" << i << ": " << info.name << ", " << info.card_name << ", " << info.device_name << std::endl;;
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
	w_param.duration = 20;
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
		auto part_size = r_file.GetAudioParams().audio_format.size_from_duration(w_param.duration);

		char buffer[10000];

		while (auto ret = r_file.Read(buffer, part_size))
		{
			w_point.Write(r_file.GetAudioParams().audio_format, buffer, ret);
			timer(w_param.duration);
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

	auto* read_audio_stream = audio_server.AddStream(player.GetAudioFormat(), alsa_session_id, false);
	auto* write_audio_stream = audio_server.AddStream(recorder.GetAudioFormat(), alsa_session_id, true);
	auto* r_file_audio_stream1 = audio_server.AddStream(r_file1.GetAudioFormat(), file_session_id1, true);
	auto* r_file_audio_stream2 = audio_server.AddStream(r_file2.GetAudioFormat(), file_session_id2, true);
	auto* w_file_audio_stream1 = audio_server.AddStream(w_file1.GetAudioFormat(), file_session_id1, false);

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

int main()
{
	// test_queue();

	// test_alsa();

	// test_media_queue();

	// test_audio_file();

	// test_audio_channel_worker();

	// test_audio_dispatcher();

	test_composer();

	return 0;
}
