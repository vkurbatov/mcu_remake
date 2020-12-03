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
#include "media/audio/volume_controller.h"

#include <mutex>
#include <cmath>
#include <cstring>

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


struct point_t
{
    double x;
    double y;

    static double distance(const point_t& p1, const point_t& p2)
    {
        return p1.distance(p2);
    }

    point_t(double x = 0.0, double y = 0.0)
        : x(x)
        , y(y)
    {

    }

    double distance(const point_t& p = {}) const
    {
        return std::sqrt(std::pow(p.x - x, 2) + std::pow(p.y - y, 2));
    }

    bool operator == (const point_t& p) const
    {
        return x == p.x && p.y == y;
    }

    bool operator != (const point_t& p) const
    {
        return !operator == (p);
    }
};

point_t abs_mouse_pos;
point_t rel_mouse_pos;

const double area_size = 10.0; // в метрах

class AudioTranciever : public core::media::audio::IAudioReader
        , public core::media::audio::IAudioWriter
{
    std::mutex                  m_mutex;
    core::media::DataQueue      m_queue;

    std::vector<point_t>        m_sensors_pos;
    std::vector<point_t>        m_stereo_pos;
    std::vector<std::uint8_t>   m_process_buffer;

public:
    AudioTranciever(std::size_t queue_size)
        : m_queue(queue_size)
        , m_sensors_pos({ {-1.0, 0.0 }, { 1.0, 0.0 }, { 0.0, 2.0 } /*, { 2.0, 0.6 }, { 0.0, 2.0 }*/ })
        , m_stereo_pos({ {-1.0, 0.0 }, { 1.0, 0.0 } })
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

        m_process_buffer.resize(size);
        auto result = m_queue.Pop(m_process_buffer.data(), size);

        if (result > 0)
        {
            filter(m_process_buffer.data(), data, result);
        }

        return result;
    }
private:
    void filter(const void *input_data, void *output_data, std::size_t size)
    {
        auto input_pcm = static_cast<const std::int16_t*>(input_data);
        auto output_pcm = static_cast<std::int16_t*>(output_data);
        auto samples = size / sizeof(std::int16_t);
        const auto end = input_pcm + samples;

        std::memset(output_pcm, 0, size);

        //core::media::audio::VolumeController::VolumeChange(core::media::audio::sample_format_t::pcm_16, 10, input_pcm, size, output_pcm, size);

        while (input_pcm < end)
        {
            for (const auto& st : m_sensors_pos)
            {
                double sample = 0;

                double max_factor = 0.5;//1.0 / static_cast<double>(m_sensors_pos.size());

                for (const auto& ss : m_sensors_pos)
                {
                    auto distance = rel_mouse_pos.distance(ss) + ss.distance(st);
                    auto factor = max_factor - distance / 10.0;

                    factor = std::max(0.0, std::min(max_factor, factor));

                    sample += static_cast<double>(*input_pcm) * factor;
                }

                *output_pcm = static_cast<std::int16_t>(sample * 0.5);
/*
                auto distance = rel_mouse_pos.distance(st);
                auto factor = 1.0 - distance / 5.0;
                if (factor < 0.0)
                {
                    factor = 0.0;
                }
                *output_pcm += static_cast<double>(*input_pcm) * factor;
*/
                input_pcm++;
                output_pcm++;
            }
        }

        return;

        // std::memset();
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
    AudioLoopback(std::uint32_t duration_ms = 20, std::uint32_t sample_rate = 32000)
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
    installEventFilter(this);
    setMouseTracking(true);
}

synth_form::~synth_form()
{
    delete ui;
}

bool synth_form::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseMove)
    {
          QMouseEvent* mouseEvent = (QMouseEvent*)event;
          if (mouseEvent->type() == QMouseEvent::MouseMove)
          {

             abs_mouse_pos.x = mouseEvent->x();
             abs_mouse_pos.y = mouseEvent->y();

             auto current_size = std::min(ui->centralwidget->width(), ui->centralwidget->height());

             double factor = area_size / current_size;

             rel_mouse_pos.x = (abs_mouse_pos.x - ui->centralwidget->width() / 2) * factor;
             rel_mouse_pos.y = (-(abs_mouse_pos.y - (ui->centralwidget->height() / 2))) * factor;

             const auto& pt = rel_mouse_pos;

             ui->lbPos->setText(QString("{%1:%2}, d = %3").arg(pt.x).arg(pt.y).arg(rel_mouse_pos.distance()));
          }
    }
    return false;
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
