#include "video_form.h"
#include "ui_video_form.h"

#include <QPainter>
#include <QPixmap>
#include <QVideoFrame>
#include <QPaintEvent>
#include "yuv_video_buffer.h"
#include "rgb_video_buffer.h"
#include "core/media/common/ffmpeg/libav_converter.h"
#include "core/media/common/ffmpeg/libav_stream_capturer.h"
#include "core/media/common/ffmpeg/libav_decoder.h"
#include "core/media/common/v4l2/v4l2_device.h"

#include <cstring>
#include <mutex>
#include <chrono>
#include <atomic>
#include <thread>

#include "media/common/utils/format_converter.h"

ffmpeg::scaling_method_t scaling_method = ffmpeg::scaling_method_t::default_method;
std::uint32_t scaling_factor = 1;

std::uint32_t decoded_delay = 0;
std::uint32_t convert_delay1 = 0;
std::uint32_t convert_delay2 = 0;

double dec_delay = 0;
double delay1 = 0;
double delay2 = 0;

double delay_factor = 0.1;


ffmpeg::libav_converter converter(ffmpeg::scaling_method_t::default_method);
std::unique_ptr<ffmpeg::libav_stream_capturer> rtsp_capturer;
std::unique_ptr<v4l2::v4l2_device> v4l2_capturer;
ffmpeg::libav_decoder decoder;
std::mutex  mutex;
std::atomic_bool image_change(false);

QImage last_image;

std::vector<std::uint8_t>           image_buffer;
ffmpeg::fragment_info_t             last_fragment_info;
std::vector<std::uint8_t>           last_frame_buffer;
std::vector<std::uint8_t>           last_output_buffer;

std::uint32_t   fps = 0;
std::uint32_t   frame_count = 0;
auto last_seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() % 1000;


video_form::video_form(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::video_form),
    m_surface(this)
{
    {
        QImage test_image("/home/user/ivcscodec/mcu_remake/resources/test_image.png");
        image_buffer.resize(test_image.width() * test_image.height() * 3);

        for (int i = 0; i < test_image.height(); i++)
        {
            uchar *src = test_image.scanLine(i);
            uchar *dst = image_buffer.data() + test_image.width() * 3 * i;

            for (int j = 0; j < test_image.width(); j++)
            {
                //std::memcpy(dst + j * 3, src + j * 4, 3);
                dst[j * 3 + 0] = src[j * 4 + 0];
                dst[j * 3 + 1] = src[j * 4 + 1];
                dst[j * 3 + 2] = src[j * 4 + 2];

            }
        }

        auto rtsp_data_handler = [this](const ffmpeg::stream_info_t& stream_info
                , ffmpeg::media_data_t&& media_data)
        {
            if (stream_info.media_type == ffmpeg::media_type_t::video)
            {
                if (!decoder.is_open())
                {
                    decoder.open(stream_info.codec_info.id
                                 , stream_info.media_info
                                 , stream_info.codec_info.extra_data.data()
                                 , stream_info.codec_info.extra_data.size());
                }

                if (decoder.is_open())
                {
                    auto tp = std::chrono::high_resolution_clock::now();

                    auto decoded_frames = std::move(decoder.decode(media_data.data(), media_data.size()));

                    decoded_delay = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - tp).count();
                    dec_delay += (decoded_delay - dec_delay) * delay_factor;

                    mutex.lock();

                    while (!decoded_frames.empty())
                    {
                        ffmpeg::decoded_frame_t& frame = decoded_frames.front();

                        std::uint32_t d_x = 0;
                        std::uint32_t d_y = 0;

                        last_fragment_info = ffmpeg::fragment_info_t(d_x
                                                             , d_y
                                                             , frame.info.media_info.video_info.size.width - d_x * 2
                                                             , frame.info.media_info.video_info.size.height - d_y * 2
                                                             , frame.info.media_info.video_info.size.width
                                                             , frame.info.media_info.video_info.size.height
                                                             , frame.info.media_info.video_info.pixel_format
                                                            );
                        last_frame_buffer = std::move(frame.media_data);

                        decoded_frames.pop();
                    }

                    mutex.unlock();

                    if (image_change == false)
                    {
                        image_change = true;
                        QMetaObject::invokeMethod(this, "on_update", Qt::QueuedConnection);
                    }
                }
            }

            return true;
        };

        rtsp_capturer.reset(new ffmpeg::libav_stream_capturer(rtsp_data_handler));


        auto v4ls_data_handler = [this](const v4l2::frame_info_t& frame_info
                , v4l2::frame_data_t&& frame_data)
        {

            auto video_format = core::media::utils::format_conversion::form_v4l2_format(frame_info.pixel_format);


            auto codec = core::media::utils::format_conversion::to_ffmpeg_codec(video_format);
            auto format = core::media::utils::format_conversion::to_ffmpeg_format(video_format);


            if (codec == ffmpeg::codec_id_raw_video
                    || codec == ffmpeg::codec_id_none)
            {
                mutex.lock();

                std::uint32_t d_x = 0;
                std::uint32_t d_y = 0;

                last_fragment_info = ffmpeg::fragment_info_t(d_x
                                                             , d_y
                                                             , frame_info.size.width - d_x * 2
                                                             , frame_info.size.height - d_y * 2
                                                             , frame_info.size.width
                                                             , frame_info.size.height
                                                             , format
                                                             );

                last_frame_buffer = std::move(frame_data);

                mutex.unlock();
            }
            else
            {
                if (decoder.is_open()
                        && codec != decoder.codec_id())
                {
                    decoder.close();
                }

                if (!decoder.is_open())
                {

                    ffmpeg::media_info_t media_info;

                    media_info.video_info.fps = frame_info.fps;
                    media_info.video_info.size = { frame_info.size.width, frame_info.size.height };
                    media_info.video_info.pixel_format = format;
                    decoder.open(codec
                                 , media_info);
                }

                if (decoder.is_open())
                {
                    auto tp = std::chrono::high_resolution_clock::now();

                    auto decoded_frames = std::move(decoder.decode(frame_data.data(), frame_data.size()));

                    decoded_delay = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - tp).count();
                    dec_delay += (decoded_delay - dec_delay) * delay_factor;

                    mutex.lock();

                    while (!decoded_frames.empty())
                    {
                        ffmpeg::decoded_frame_t& frame = decoded_frames.front();

                        std::uint32_t d_x = 0;
                        std::uint32_t d_y = 0;

                        last_fragment_info = ffmpeg::fragment_info_t(d_x
                                                                     , d_y
                                                                     , frame.info.media_info.video_info.size.width - d_x * 2
                                                                     , frame.info.media_info.video_info.size.height - d_y * 2
                                                                     , frame.info.media_info.video_info.size.width
                                                                     , frame.info.media_info.video_info.size.height
                                                                     , frame.info.media_info.video_info.pixel_format
                                                                     );

                        last_frame_buffer = std::move(frame.media_data);

                        decoded_frames.pop();
                    }

                    mutex.unlock();

                }
            }

            if (image_change == false)
            {
                image_change = true;
                QMetaObject::invokeMethod(this, "on_update", Qt::QueuedConnection);
            }

            return true;
        };

        v4l2_capturer.reset(new v4l2::v4l2_device(v4ls_data_handler));
    }
    ui->setupUi(this);
}

video_form::~video_form()
{
    rtsp_capturer->close();
    v4l2_capturer->close();
    delete ui;
}

void video_form::prepare_image()
{
    mutex.lock();

    ffmpeg::fragment_info_t input_info = last_fragment_info;
    std::vector<std::uint8_t> input_buffer = std::move(last_frame_buffer);

    mutex.unlock();

    auto current_seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() % 1000;

    if (!input_buffer.empty())
    {
        ffmpeg::fragment_info_t output_info = input_info;
        output_info.pixel_format = ffmpeg::pixel_format_rgb24;

        output_info.frame_rect.size = output_info.frame_size = { 1920, 1080 };

        const QSize q_size(output_info.frame_size.width
                           , output_info.frame_size.height);

        std::vector<std::uint8_t> output_buffer(output_info.get_frame_size(), 0);

        /*
        const auto w_cnt = 16;
        const auto h_cnt = 16;

        const auto w_step = q_size.width() / w_cnt;
        const auto h_step = q_size.height() / h_cnt;

        output_info.frame_rect.size = input_info.frame_rect.size = { w_step, h_step };

        for (int i = 0; i < w_cnt; i++)
        {
            for (int j = 0; j < h_cnt; j++)
            {
                auto x = i * w_step;
                auto y = j * h_step;

                output_info.frame_rect.offset = input_info.frame_rect.offset = { x, y };
                output_info.frame_rect.offset = { q_size.width() - x - w_step, q_size.height() - y - h_step };

                converter.convert(input_info
                                  , frame.media_data.data()
                                  , output_info
                                  , output_buffer.data()
                                  , (i + y) % 2 == 0);
            }
        }*/

        /*
        const auto w_cnt = 10;
        const auto h_cnt = 10;

        const auto w_step = q_size.width() / w_cnt;
        const auto h_step = q_size.height() / h_cnt;

        output_info.frame_rect.size = { w_step, h_step };

        for (int i = 0; i < w_cnt; i++)
        {
            for (int j = 0; j < h_cnt; j++)
            {
                auto x = i * w_step;
                auto y = j * h_step;

                //output_info.frame_rect.offset = input_info.frame_rect.offset = { x, y };
                output_info.frame_rect.offset = { q_size.width() - x - w_step, q_size.height() - y - h_step };

                converter.convert(input_info
                                  , frame.media_data.data()
                                  , output_info
                                  , output_buffer.data()
                                  , (i + y) % 2 == 0);
            }
        }*/

        const auto k_w = scaling_factor;
        const auto k_h = scaling_factor;

        ffmpeg::fragment_info_t mid_info = input_info;

        mid_info.frame_rect.size.width /= k_w;
        mid_info.frame_rect.size.height /= k_h;
        mid_info.frame_size = mid_info.frame_rect.size;
        mid_info.pixel_format = output_info.pixel_format;

        std::vector<std::uint8_t> mid_buffer(mid_info.get_frame_size());

        if (scaling_method != converter.scaling_method())
        {
            converter.reset(scaling_method);
        }

        auto tp = std::chrono::high_resolution_clock::now();

        auto res = converter.convert(input_info
                          , input_buffer.data()
                          , mid_info
                          , mid_buffer.data()
                          , false);

        convert_delay1 = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - tp).count();
        tp = std::chrono::high_resolution_clock::now();

        res = converter.convert(mid_info
                          , mid_buffer.data()
                          , output_info
                          , output_buffer.data()
                          , false);

        convert_delay2 = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - tp).count();

        delay1 += (convert_delay1 - delay1) * delay_factor;
        delay2 += (convert_delay2 - delay2) * delay_factor;

        last_output_buffer = std::move(output_buffer);
        last_image = QImage(last_output_buffer.data(), q_size.width(), q_size.height(), QImage::Format_RGB888);

        frame_count++;

        if (current_seconds != last_seconds)
        {
            fps = frame_count;
            frame_count = 0;
            last_seconds = current_seconds;
        }
    }

    image_change = false;
}

void video_form::on_pushButton_clicked()
{

    auto& device = v4l2_capturer;

    if (device->is_opened())
    {
        device->close();
    }
    else
    {
        //rtsp_capturer->open("rtsp://admin:Algont12345678@10.11.4.151");
        //rtsp_capturer->open("camera://dev/video0");
        device->open("/dev/video2", 4);
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        auto formats = device->get_supported_formats();

        ui->cbResoulution->clear();

        for (const auto& f : formats)
        {
            auto video_format = core::media::utils::format_conversion::form_v4l2_format(f.pixel_format);

            auto item_string = QString("%1x%2@%3:%4").arg(QString::number(f.size.width)
                                                          , QString::number(f.size.height)
                                                          , QString::number(f.fps)
                                                          , QString::fromStdString(core::media::utils::format_conversion::get_format_name(video_format)));
            ui->cbResoulution->addItem(item_string);
        }

        ui->cbControlList->clear();

        auto controls = device->get_control_list();
        for (const auto& c : controls)
        {

            auto item_string = QString("%1").arg(QString::fromStdString(c.name));

            ui->cbControlList->addItem(item_string);
        }

    }

    ui->pushButton->setText(device->is_opened() ? "Stop" : "Start");
    return;

    QSize size_src(1280, 720);
    QSize size_dst(1280, 720);

    std::vector<std::uint8_t> buffer1(size_dst.width() * size_dst.height() * 3);

    std::vector<std::uint8_t> yuv_input_buffer((size_src.width() * size_src.height() * 12) / 8);
    // std::vector<std::uint8_t> yuv_output_buffer((size_dst.width() * size_dst.height() * 12) / 8);

    // buffer1.resize(buffer1.size() * 2);

    auto buffer2 = image_buffer;

    std::memset(buffer1.data(), 200, buffer1.size());
    //std::memset(buffer2.data(), 50, buffer2.size());

    const auto s_x = 360;
    const auto s_y = 240;

    const auto d_x = 360;
    const auto d_y = 240;

    ffmpeg::fragment_info_t brg_info(0, 0, size_src.width(), size_src.height(),  size_src.width(), size_src.height(), ffmpeg::pixel_format_bgr24);

    ffmpeg::fragment_info_t yuv_info(0, 0, size_src.width(), size_src.height(), size_src.width(), size_src.height(), ffmpeg::pixel_format_yuv420p);

    ffmpeg::fragment_info_t input_fragment_info(s_x, s_y, size_src.width() - s_x * 2, size_src.height() - s_y * 2, size_src.width(), size_src.height(), ffmpeg::pixel_format_yuv420p);

    // ffmpeg::fragment_info_t input_fragment_info(s_x, s_y, size_src.width() - s_x * 2, size_src.height() - s_y * 2, size_dst.width(), size_dst.height(), ffmpeg::pixel_format_rgb24);

    ffmpeg::fragment_info_t output_fragment_info(d_x, d_y, size_dst.width() - d_x * 2, size_dst.height() - d_y * 2, size_dst.width(), size_dst.height(), ffmpeg::pixel_format_rgb24);

    converter.convert( brg_info
                      , buffer2.data()
                      , yuv_info
                      , yuv_input_buffer.data()
                      , false);

    converter.convert( input_fragment_info
                      , yuv_input_buffer.data()
                      , output_fragment_info
                      , buffer1.data()
                      , true);

    /*converter.convert(input_fragment_info
                      , buffer2.data()
                      , output_fragment_info
                      , buffer2.data());*/


    QVideoFrame frame(new rgb_video_buffer(size_dst, buffer1.data()), size_dst, QVideoFrame::PixelFormat::Format_RGB24);

    m_surface.present(frame);
    // close();
}

void video_form::on_update()
{
    /*
    mutex.lock();
    m_surface.present(last_frame);   
    mutex.unlock();*/
    prepare_image();
    repaint();
}

void video_form::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    {
        mutex.lock();
        for (const auto& rect : event->region().rects())
        {
            //painter.drawImage();
            painter.drawImage(rect, last_image, rect);
        }
        mutex.unlock();
    }

    painter.setPen(QPen(Qt::green, 1, Qt::SolidLine, Qt::FlatCap));
    painter.drawText(rect(), QString("%1/%2/%3/%4")
                     .arg(QString::number(dec_delay / 1000, 'f', 2))
                     .arg(QString::number(delay1 / 1000, 'f', 2))
                     .arg(QString::number(delay2 / 1000, 'f', 2))
                     .arg(QString::number(fps)));

}

void video_form::on_cbScalingMethod_currentIndexChanged(int index)
{
    scaling_method = static_cast<ffmpeg::scaling_method_t>(index == 0 ? 0 : 1 << (index - 1));
}

void video_form::on_cbScaling_currentIndexChanged(int index)
{
    const std::uint32_t scaling_table[] = { 1, 2, 4, 8, 10 };
    scaling_factor = scaling_table[index];
}

void video_form::on_cbResoulution_activated(const QString &arg1)
{

}

void video_form::on_cbResoulution_activated(int index)
{
    auto formats = v4l2_capturer->get_supported_formats();
    if (index >= 0 && index < formats.size())
    {
        v4l2_capturer->set_format(formats[index]);
    }
    // for
}

void video_form::on_cbControlList_activated(int index)
{
    auto controls = v4l2_capturer->get_control_list();
    if (index >= 0 && index < controls.size())
    {
        v4l2::control_t& ctrl = controls[index];
        ui->slControl->setMaximum(ctrl.range.max);
        ui->slControl->setMinimum(ctrl.range.min);
        ui->slControl->setValue(ctrl.current_value);
    }
}

void video_form::on_slControl_actionTriggered(int action)
{

}

void video_form::on_slControl_sliderMoved(int position)
{
    auto controls = v4l2_capturer->get_control_list();
    auto index = ui->cbControlList->currentIndex();

    if (index >= 0 && index < controls.size())
    {
        v4l2::control_t& ctrl = controls[index];
        v4l2_capturer->set_control(ctrl.id, position);
    }
}
