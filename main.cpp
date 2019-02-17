#include <iostream>
#include <thread>
#include <cstring>
#include <mutex>

#include "core/media/audio/channels/alsa/alsa_channel.h"
#include "core/media/common/data_queue.h"
#include "core/media/audio/audio_resampler.h"

int main()
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
	static const std::uint32_t recorder_sample_rate = 32000;
	static const std::uint32_t recorder_frame_size = 2 * (recorder_sample_rate * duration_ms) / 1000;
	static const std::uint32_t playback_sample_rate = 48000;
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
		core::media::audio::channels::audio_channel_params_t player_params(core::media::audio::channels::channel_direction_t::playback, { 48000, core::media::audio::audio_format_t::sample_format_t::pcm_16, 1 }, duration_ms, true);

		core::media::audio::channels::alsa::AlsaChannel player(player_params);

		player.Open("default");

		player.SetVolume(100);


		std::uint8_t buffer[recorder_frame_size];

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

				player.Write(buffer, q_size, input_format);

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

    return 0;
}
