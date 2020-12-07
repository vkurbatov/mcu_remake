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

double wave_speed = 331.0;
double wave_delay = 1.0 / wave_speed;

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
bool phase_process = false;
const std::vector<point_t> default_sensor_pos = { {-2.0, -0.5 }, { 2.0, -0.5 }, { 0.0, 2.0 }, { -1.5, 1.7 }, { 1.5, 1.7 } };

const double area_size = 10.0; // в метрах

/*
class AudioSensor
{
    point_t                             m_position;
    std::vector<std::uint8_t>           m_audio_buffer;
    core::media::audio::audio_info_t    m_audio_format;
public:
    AudioSensor(const point_t& position)
        : m_position(position)
        , m_audio_format(core::media::audio::sample_format_t::pcm_16, 32000, 1)
    {

    }

    void Write(const void* data, std::size_t size)
    {

        if (m_audio_buffer.size() < size * 2)
        {
            m_audio_buffer.resize(size * 2);
        }

        std::memcpy(m_audio_buffer.data(), m_audio_buffer.data() + size, size);
        std::memcpy(m_audio_buffer.data() + size, data, size);
    }

    std::size_t Read(const point_t& pos, void* data, std::size_t size)
    {
        double distance = rel_mouse_pos.distance(m_position) + m_position.distance(pos);
        std::int32_t samples_delay = m_audio_format.samples_from_duration(static_cast<std::int32_t>(distance * wave_delay));

        if (m_audio_buffer.size() > size)
        {
            auto pcm_input = static_cast<const std::int16_t*>(m_audio_buffer.data() + size - samples_delay * 2);
            auto pcm_output = static_cast<std::int16_t*>(data);
            auto samples = size / 2;

            double max_factor = 0.5;

            while (samples-->0)
            {
                auto factor = max_factor - distance / 10.0;

                factor = std::max(0.0, std::min(max_factor, factor));

                *pcm_output++ = pcm_input++ ;
            }

            std::memcpy(data, m_audio_buffer.data() + size - samples_delay * 2, size);
            return size;
        }

        return 0;
    }
};

*/

class AudioTranciever : public core::media::audio::IAudioReader
        , public core::media::audio::IAudioWriter
{
    std::mutex                  m_mutex;
    // core::media::DataQueue      m_queue;
    std::vector<std::uint8_t>   m_buffer;

    std::vector<point_t>        m_sensors_pos;
    std::vector<point_t>        m_stereo_pos;
    std::vector<std::uint8_t>   m_process_buffer;

public:
    AudioTranciever(std::size_t queue_size)
         //m_queue(queue_size)
        : m_sensors_pos(default_sensor_pos)
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

        if (m_buffer.size() < size * 2)
        {
            m_buffer.resize(size * 2, 0);
        }
        std::memcpy(m_buffer.data(), m_buffer.data() + size, size);
        std::memcpy(m_buffer.data() + size, data, size);

        //return m_queue.Push(data, size);
    }

    // IAudioReader interface
public:
    int32_t Read(const core::media::audio::audio_format_t &audio_format, void *data, std::size_t size, uint32_t options)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_buffer.size() > size)
        {
            filter(m_buffer.data() + size, data, size);
            return size;
        }

        return 0;
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
            for (const auto& st : m_stereo_pos)
            {
                double sample = 0;

                double max_factor = 0.5;//1.0 / static_cast<double>(m_sensors_pos.size());

                for (const auto& ss : m_sensors_pos)
                {
                    auto distance = rel_mouse_pos.distance(ss) + ss.distance(st);
                    auto delay_ms = phase_process ? static_cast<std::int32_t>(distance * wave_delay * 1000.0) : 0;

                    std::int32_t samples_delay = core::media::audio::audio_info_t(core::media::audio::sample_format_t::pcm_16, 32000, 1).samples_from_duration(delay_ms);

                    //auto factor = max_factor - distance / 10.0;
                    auto factor = std::exp(-distance / 1.5);

                    factor = std::max(0.0, std::min(max_factor, factor));

                    sample += static_cast<double>(*(input_pcm - samples_delay)) * factor;
                }

                *output_pcm = static_cast<std::int16_t>(sample * 0.5);

                input_pcm++;
                output_pcm++;
            }
        }

        return;
        // std::memset();
    }
};

class AudioGenerator : public core::media::audio::IAudioReader
{
    std::int32_t    m_step;
    double          m_freq;
    double          m_level;
    // IMediaReadStatus interface
public:
    AudioGenerator(double freq = 300.0
            , double level = 0.2)
        : m_step(0)
        , m_freq(freq)
        , m_level(level)
    {

    }
public:
    bool CanRead() const
    {
        return true;
    }

    // IAudioReader interface
public:
    int32_t Read(const core::media::audio::audio_format_t &audio_format
                 , void *data
                 , std::size_t size
                 , uint32_t options)
    {

        auto period = static_cast<double>(audio_format.sample_rate) / m_freq / 2;
        double factor = M_PI / period;

        auto pcm = static_cast<std::int16_t*>(data);
        auto samples = size / sizeof(*pcm);

        while(samples-- > 0)
        {
            auto sample = m_level * std::sin(static_cast<double>(m_step) * factor);
            *pcm = static_cast<double>(sample * 32767.0);

            pcm++;
            m_step++;
        }

        return size;
    }
};

class AudioLoopback
{
    std::uint32_t m_duration;
    core::media::audio::channels::alsa::AlsaChannel m_recorder;
    core::media::audio::channels::alsa::AlsaChannel m_playback;
    AudioTranciever                     m_transiever;
    core::media::audio::AudioDispatcher m_reader;
    AudioGenerator                      m_generator;
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


double resize_factor = 0.0;
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

             resize_factor = area_size / current_size;

             rel_mouse_pos.x = (abs_mouse_pos.x - ui->centralwidget->width() / 2) * resize_factor;
             rel_mouse_pos.y = (-(abs_mouse_pos.y - (ui->centralwidget->height() / 2))) * resize_factor;

             const auto& pt = rel_mouse_pos;

             ui->lbPos->setText(QString("{%1:%2}, d = %3").arg(pt.x).arg(pt.y).arg(rel_mouse_pos.distance()));
          }
    }
    return false;
}

void synth_form::paintEvent(QPaintEvent *pe)
{
    QPainter p(this);
    QPen pen;
    pen.setColor(Qt::black);
    p.setPen(pen);
    p.setBrush(Qt::black);


    auto w = ui->centralwidget->width();
    auto h = ui->centralwidget->height();

    auto x = w / 2;
    auto y = h / 2;

    p.drawEllipse({ x, y }, 3, 3);

    auto points = default_sensor_pos;

    for (const auto& pt : points)
    {
        x = pt.x / resize_factor + w / 2;
        y = (-pt.y) / resize_factor + h / 2;
        p.drawEllipse({ x, y }, 3, 3);
    }
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

void synth_form::on_cbPhase_stateChanged(int arg1)
{
    phase_process = arg1 > 0;
}
