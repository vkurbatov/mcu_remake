#include "synth_form.h"
#include "ui_synth_form.h"

#include <QPainter>
#include <QPixmap>
#include <QVideoFrame>
#include <QPaintEvent>

#include <QVariant>

#include "media/audio/audio_dispatcher.h"
#include "media/audio/channels/alsa/alsa_channel.h"
#include "media/common/data_queue.h"

#include <mutex>

/*
    static const std::uint32_t duration_ms = 10;
    static const std::uint32_t recorder_sample_rate = 32000;
    static const std::uint32_t playback_sample_rate = 48000;

    core::media::audio::channels::audio_channel_params_t recorder_params(core::media::audio::channels::channel_direction_t::recorder, { recorder_sample_rate, core::media::audio::sample_format_t::pcm_16, 1 }, duration_ms, true);

    core::media::audio::channels::alsa::AlsaChannel recorder(recorder_params);

    core::media::audio::channels::audio_channel_params_t player_params(core::media::audio::channels::channel_direction_t::playback, { playback_sample_rate, core::media::audio::sample_format_t::pcm_16, 1 }, duration_ms, true);

    core::media::audio::channels::alsa::AlsaChannel player(player_params);

    core::media::audio::AudioDispatcher dispatcher(recorder, player, player_params.audio_format);

    core::media::DelayTimer timer;

    recorder.Open("default");
    player.Open("default");

    dispatcher.Start(duration_ms);

    while(dispatcher.IsRunning()) timer(duration_ms);
*/

class AudioTranciever : public core::media::audio::IAudioReader
        , public core::media::audio::IAudioWriter
{
    std::mutex                  m_mutex;
    core::media::DataQueue      m_queue;

public:
    AudioTranciever(std::size_t queue_size)
        : m_queue(queue_size)
    {

    }

    // IMediaWriteStatus interface
public:
    bool CanWrite() const
    {
        return true;
    }

    // IMediaReadStatus interface
public:
    bool CanRead() const
    {
        return true;
    }

    // IAudioWriter interface
public:
    int32_t Write(const core::media::audio::audio_format_t &audio_format, const void *data, std::size_t size, uint32_t options)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.Push(data, size);
    }

    // IAudioReader interface
public:
    int32_t Read(const core::media::audio::audio_format_t &audio_format, void *data, std::size_t size, uint32_t options)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto result = m_queue.Pop(data, size);

        auto pcm = static_cast<std::uint16_t*>(data);
        auto samples = size / sizeof(pcm[0]);

        for (auto i = 0; i < samples; i++)
        {
            if (i % 2 != 0)
            {
                pcm[i] = 0;
            }
        }

        return result;
    }
};

class AudioLoopback
{
    std::uint32_t m_duration;
    core::media::audio::channels::alsa::AlsaChannel m_recorder;
    core::media::audio::channels::alsa::AlsaChannel m_playback;
    AudioTranciever m_transiever;
    core::media::audio::AudioDispatcher m_reader;
    core::media::audio::AudioDispatcher m_writer;

public:
    AudioLoopback(std::uint32_t duration_ms = 10, std::uint32_t sample_rate = 32000)
        : m_duration(duration_ms)
        , m_recorder({core::media::audio::channels::channel_direction_t::recorder, { sample_rate, core::media::audio::sample_format_t::pcm_16, 1 }, duration_ms})
        , m_playback({core::media::audio::channels::channel_direction_t::playback, { sample_rate, core::media::audio::sample_format_t::pcm_16, 2 }, duration_ms})
        , m_transiever(m_playback.GetAudioParams().audio_format.size_from_duration(duration_ms) * 10)
        , m_reader(m_recorder, m_transiever, m_playback.GetAudioParams().audio_format)
        , m_writer(m_transiever, m_playback, m_playback.GetAudioParams().audio_format)
    {

    }

    void play()
    {
        if (!is_started())
        {
            m_recorder.Open("default");
            m_playback.Open("default");
            m_reader.Start(m_duration);
            m_writer.Start(m_duration);
        }
    }
    void stop()
    {
        if (is_started())
        {
            m_writer.Stop();
            m_reader.Stop();
            m_playback.Close();
            m_recorder.Close();
        }
    }
    bool is_started()
    {
        return m_reader.IsRunning() || m_writer.IsRunning();
    }
};

AudioLoopback audio_loopback;

synth_form::synth_form(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::synth_form)
{   
    ui->setupUi(this);
}

synth_form::~synth_form()
{
    delete ui;
}

void synth_form::on_btPlay_clicked()
{
    if (!audio_loopback.is_started())
    {
        audio_loopback.play();
    }
    else
    {
        audio_loopback.stop();
    }

    ui->btPlay->setText(audio_loopback.is_started() ? "Stop" : "Play");
}
