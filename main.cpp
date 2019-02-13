#include <iostream>
#include <thread>
#include <cstring>
#include <mutex>

#include "core/media/audio/devices/alsa_device.h"
#include "core/media/common/data_queue.h"
#include "core/media/audio/audio_resampler.h"

int main()
{

    int i = 0;

    auto device_playback_list = core::media::audio::devices::AlsaDevice::GetDeviceList(false, "plughw");
    auto device_recorder_list = core::media::audio::devices::AlsaDevice::GetDeviceList(true, "plughw");

	std::cout << "ALSA playback list " << device_playback_list.size() << ":" << std::endl;

	for (const auto& info : device_playback_list)
    {
		std::cout << "#" << i << ": " << info.name << ", " << info.description << std::endl;
        i++;
    }

	i = 0;

	std::cout << "ALSA recorder list " << device_recorder_list.size() << ":" << std::endl;

	for (const auto& info : device_recorder_list)
	{
		std::cout << "#" << i << ": " << info.name << ", " << info.description << std::endl;;
		i++;
	}


    static const std::uint32_t duration_ms = 10;
    static const std::uint32_t sample_rate = 32000;
    static const std::uint32_t frame_size = 2 * (sample_rate * duration_ms) / 1000;
    static const std::uint32_t buffers_count = 10;

    using clock = std::chrono::high_resolution_clock;

    core::media::common::DataQueue queue(frame_size * buffers_count);
    std::mutex  queue_mutex;

    std::thread recorder_thread([&device_recorder_list, &queue_mutex, &queue]()
    {
        core::media::audio::devices::audio_params_t recorder_params(true, { sample_rate, 16, 1 }, frame_size, false);

        core::media::audio::devices::AlsaDevice recorder;

        recorder.Open("default", recorder_params);

        // recorder.SetVolume(1000);

        std::uint8_t buffer[frame_size];

        auto start = std::chrono::high_resolution_clock::now();

        while (recorder.IsOpen())
        {
            auto t_0 = clock::now();

            auto ret = recorder.Read(buffer, frame_size);

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
        core::media::audio::devices::audio_params_t player_params(false, { 48000, 16, 1 }, frame_size * 2, true);

        core::media::audio::devices::AlsaDevice player;

        player.Open("default", player_params);

        player.SetVolume(100);

        std::uint8_t buffer[frame_size];

        std::uint8_t resample_buffer[frame_size * 2];

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        auto start = clock::now();

        core::media::audio::audio_format_t input_format(sample_rate, 16, 1);

        while (player.IsOpen())
        {

            auto t_0 = clock::now();

            std::size_t ret = 0;
            {
                std::lock_guard<decltype(queue_mutex)> gl(queue_mutex);

                ret = queue.Pop(buffer, frame_size);
            }

            ret = core::media::audio::AudioResampler::Resampling(input_format, player_params.audio_format, buffer, ret, resample_buffer);

            ret = player.Write(resample_buffer, ret);

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
