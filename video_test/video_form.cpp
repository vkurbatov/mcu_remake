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
#include "media/common/utils/format_converter.h"
#include "media/video/video_frame.h"
#include "media/common/media_frame.h"
#include "media/video/video_frame_converter.h"
#include "media/video/filters/video_filter_flip.h"
#include "media/common/opencv/cv_base.h"
#include "media/common/magick/magick_base.h"
#include "media/common/qt/qt_base.h"

#include "media/video/filters/video_filter_overlay.h"
#include "media/video/filters/video_layer_text.h"
#include "media/video/filters/video_layer_image.h"
#include "media/video/filters/video_layer_figure.h"

#include <cstring>
#include <mutex>
#include <chrono>
#include <atomic>
#include <thread>

ffmpeg::scaling_method_t scaling_method = ffmpeg::scaling_method_t::default_method;
core::media::video::aspect_ratio_mode_t aspect_ratio_method = core::media::video::aspect_ratio_mode_t::scale;

std::uint32_t scaling_factor = 1;

std::uint32_t decoded_delay = 0;
std::uint32_t convert_delay1 = 0;
std::uint32_t convert_delay2 = 0;

double dec_delay = 0;
double delay1 = 0;
double delay2 = 0;
double filter_delay = 0;

double delay_factor = 0.1;


ffmpeg::libav_converter converter(ffmpeg::scaling_method_t::default_method);
std::unique_ptr<ffmpeg::libav_stream_capturer> rtsp_capturer;
std::unique_ptr<v4l2::v4l2_device> v4l2_capturer;
ffmpeg::libav_decoder decoder;
std::mutex  mutex;
std::atomic_bool image_change(false);

core::media::video::video_frame_converter           frame_converter(scaling_method);
core::media::video::filters::video_filter_flip      filter_flip;

core::media::video::filters::layer_list_t         overlay_list;
core::media::video::filters::video_filter_overlay   filter_overlay(overlay_list);

//core::media::video::filters::video_filter_custom    filter_custom;
//core::media::video::filters::video_drawing_text_filter  filter_text("Hello world!!!", { 100, 100 });


QImage last_image;

std::vector<std::uint8_t>           image_buffer;
ffmpeg::fragment_info_t             last_fragment_info;
std::vector<std::uint8_t>           last_frame_buffer;
std::vector<std::uint8_t>           last_output_buffer;

core::media::video::frame_size_t    draw_image_size(640, 360);
std::vector<std::uint8_t>           draw_image_buffer(draw_image_size.size() * 3);

std::uint32_t   fps = 0;
std::uint32_t   frame_count = 0;
std::uint32_t   real_fps = 0;
std::uint32_t   real_frame_count = 0;

auto last_seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() % 1000;
auto real_last_seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() % 1000;


video_form::video_form(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::video_form),
    m_surface(this)
{

    QImage test_image("/home/user/ivcscodec/mcu_remake/resources/test_image.png");
    test_image = test_image.convertToFormat(QImage::Format_RGB888);

    QImage dst(draw_image_buffer.data(), draw_image_size.width, draw_image_size.height, QImage::Format_RGB888);


    QPainter painter(&dst);
    painter.drawImage(dst.rect(), test_image);
    //painter.drawImage( 0, 0, test_image);


    // test1();

    {
        // QImage test_image("/home/user/ivcscodec/mcu_remake/resources/test_image.png");


        // image_buffer.resize(test_image.width() * test_image.height() * 3);
/*
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
*/
        auto process_data = [](const ffmpeg::stream_info_t& stream_info
                          , ffmpeg::media_data_t&& media_data)
        {

            if (stream_info.media_type == ffmpeg::media_type_t::video)
            {
                auto current_seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() % 1000;

                if (stream_info.codec_info.id == ffmpeg::codec_id_raw_video
                        || stream_info.codec_info.id == ffmpeg::codec_id_none)
                {
                    mutex.lock();

                    std::uint32_t d_x = 0;
                    std::uint32_t d_y = 0;

                    last_fragment_info = ffmpeg::fragment_info_t(d_x
                                                                 , d_y
                                                                 , stream_info.media_info.video_info.size.width - d_x * 2
                                                                 , stream_info.media_info.video_info.size.height - d_y * 2
                                                                 , stream_info.media_info.video_info.size.width
                                                                 , stream_info.media_info.video_info.size.height
                                                                 , stream_info.media_info.video_info.pixel_format
                                                                 );

                    last_frame_buffer = std::move(media_data);

                    mutex.unlock();
                }
                else
                {
                    if (decoder.is_open()
                            && stream_info.codec_info.id != decoder.codec_id())
                    {
                        decoder.close();
                    }

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

                        real_frame_count++;

                        if (current_seconds != real_last_seconds)
                        {
                            real_fps = real_frame_count;
                            real_frame_count = 0;
                            real_last_seconds = current_seconds;
                        }

                        mutex.unlock();

                    }
                }
                return true;
            }
            return false;
        };


        auto rtsp_data_handler = [this, &process_data](const ffmpeg::stream_info_t& stream_info
                , ffmpeg::media_data_t&& media_data)
        {
            if (process_data(stream_info
                             , std::move(media_data)))
            {
                if (image_change == false)
                {
                    image_change = true;
                    QMetaObject::invokeMethod(this, "on_update", Qt::QueuedConnection);
                }
            }

            return true;
        };

        rtsp_capturer.reset(new ffmpeg::libav_stream_capturer(rtsp_data_handler));


        auto v4ls_data_handler = [this, &process_data](const v4l2::frame_info_t& frame_info
                , v4l2::frame_data_t&& frame_data)
        {

            auto video_format = core::media::utils::format_conversion::from_v4l2_format(frame_info.pixel_format);

            auto codec = core::media::utils::format_conversion::to_ffmpeg_codec(video_format);
            auto format = core::media::utils::format_conversion::to_ffmpeg_format(video_format);

            ffmpeg::stream_info_t stream_info;

            stream_info.codec_info.id = codec;
            stream_info.media_type = ffmpeg::media_type_t::video;
            stream_info.stream_id = 0;
            stream_info.media_info.video_info.fps = frame_info.fps;
            stream_info.media_info.video_info.size = { frame_info.size.width, frame_info.size.height };
            stream_info.media_info.video_info.pixel_format = format;

            if (process_data(stream_info
                             , std::move(frame_data)))
            {
                if (image_change == false)
                {
                    image_change = true;
                    QMetaObject::invokeMethod(this, "on_update", Qt::QueuedConnection);
                }
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
/*
    input_info.pixel_format = ffmpeg::pixel_format_yuv420p;

    std::vector<std::uint8_t> input_buffer(input_info.get_frame_size());

    converter.convert(last_fragment_info
                      , last_frame_buffer.data()
                      , input_info
                      , input_buffer.data()
                      , false);*/

    ffmpeg::fragment_info_t mid_info = input_info;
    ffmpeg::fragment_info_t output_info = input_info;

    auto margin = ui->spMargin->value();

    input_info.frame_rect.offset.x += margin;
    input_info.frame_rect.offset.y += margin;
    input_info.frame_rect.size.width -= margin * 2;
    input_info.frame_rect.size.height -= margin * 2;

    std::vector<std::uint8_t> input_buffer = std::move(last_frame_buffer);

    mutex.unlock();

    auto current_seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() % 1000;

    if (!input_buffer.empty())
    {
        output_info.pixel_format = core::media::utils::format_conversion::to_ffmpeg_format(core::media::video::pixel_format_t::rgba32);

        output_info.frame_rect.size = output_info.frame_size = { 1280, 720 };

        mid_info = output_info;

        output_info.frame_rect.offset.x += margin * 2;
        output_info.frame_rect.offset.y += margin * 2;
        output_info.frame_rect.size.width -= margin * 4;
        output_info.frame_rect.size.height -= margin * 4;

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

        mid_info.frame_rect.size.width /= k_w;
        mid_info.frame_rect.size.height /= k_h;
        mid_info.frame_size = mid_info.frame_rect.size;
        mid_info.pixel_format = ffmpeg::pixel_format_yuv420p;


        //std::vector<std::uint8_t> mid_buffer(mid_info.get_frame_size());

        if (scaling_method != converter.scaling_method())
        {
            converter.reset(scaling_method);
            frame_converter.set_scaling_method(scaling_method);
        }

        auto flip_method = static_cast<core::media::video::filters::flip_method_t>(ui->cbFlipMethod->currentIndex());

        if (flip_method != filter_flip.flip_method())
        {
            filter_flip.set_flip_method(flip_method);
        }


        core::media::video::video_format_t input_format( core::media::utils::format_conversion::from_ffmpeg_format(input_info.pixel_format)
                                                  , { input_info.frame_size.width, input_info.frame_size.height });

        auto input_frame = core::media::video::video_frame::create(input_format
                                                                   , core::media::media_buffer::create(std::move(input_buffer)));


        auto tp = std::chrono::high_resolution_clock::now();

        core::media::video::video_format_t mid_format(input_format);
        mid_format.size = { mid_info.frame_size.width, mid_info.frame_size.height };
        mid_format.pixel_format = core::media::video::pixel_format_t::yuv420p;

        /*core::media::video::frame_rect_t input_area = core::media::video::frame_rect_t( { input_info.frame_rect.offset.x, input_info.frame_rect.offset.y }
                                                                                        , frame_sizeinput_info.frame_rect.size.width, input_info.frame_rect.size.height });
*/
        core::media::video::frame_rect_t input_area(input_info.frame_rect.offset.x
                                                    , input_info.frame_rect.offset.y
                                                    , input_info.frame_rect.size.width
                                                    , input_info.frame_rect.size.height);

        frame_converter.set_input_area(input_area);
        frame_converter.set_aspect_ratio_mode(aspect_ratio_method);       


        /*auto buffer = core::media::media_buffer::create(nullptr, mid_format.plane_sizes());

        auto frame = core::media::video::video_frame::create(mid_format
                                                             , std::move(buffer));*/

        auto mid_frame = frame_converter.convert(*input_frame
                                                 , mid_format);


        filter_flip.filter(*mid_frame);


        /*auto res = converter.convert(input_info
                          , frame.planes().front()->data()
                          , mid_info
                          , mid_buffer.data()
                          , false);*/

        // void * const slices[] = { frame.planes()[0]->data(), frame.planes()[1]->data(), frame.planes()[2]->data() };

        /*auto res = converter.convert_frames(input_info
                          , input_frame->planes()[0]->data()
                          , mid_info
                          , mid_buffer.data()
                          , false);*/

        convert_delay1 = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - tp).count();
        tp = std::chrono::high_resolution_clock::now();
/*
        mid_info.frame_rect.offset.x += 40;
        mid_info.frame_rect.offset.y += 40;
        mid_info.frame_rect.size.width -= 80;
        mid_info.frame_rect.size.height -= 80;
*/

        // filter_custom.filter(*mid_frame);

        core::media::video::filters::text_format_t text_format("Times"
                                                               , 0x0000004F
                                                               , 20
                                                               , 10
                                                               , true);

        core::media::video::filters::image_decriptor_t image(draw_image_buffer.data()
                                                             , draw_image_size
                                                             , 0.5);

        core::media::video::filters::figure_format_t figure_format(core::media::video::filters::figure_type_t::polygon
                                                                   , 0x00FF007F
                                                                   , 0x0000FF7F
                                                                   , 3);

        core::media::video::filters::polyline_list_t polylines;

        if (overlay_list.empty())
        {

            polylines.push_back( { 150, 150 } );
            polylines.push_back( { 300, 150 } );
            polylines.push_back( { 225, 300 } );

            overlay_list.emplace_back(new core::media::video::filters::video_layer_image(image
                                                                                                   , { 100, 100 }));

            overlay_list.emplace_back(new core::media::video::filters::video_layer_figure(figure_format
                                                                                          , polylines));
/*
            overlay_list.emplace_back(new core::media::video::filters::video_layer_image(image
                                                                                       , { 100, 100 }));*/

            overlay_list.emplace_back(new core::media::video::filters::video_layer_text("Hello World!!!"
                                                                                       , text_format
                                                                                       , { 917, 100 }));

            text_format.color = 0xFF00009F;
            overlay_list.emplace_back(new core::media::video::filters::video_layer_text("Hello World!!!"
                                                                                       , text_format
                                                                                       , { 1117, 300 }));

        }

        filter_overlay.filter(*mid_frame);

        auto flt_delay = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()
                                                                               - tp).count();

        auto res = converter.convert_frames(mid_info
                          , mid_frame->planes()[0]->data()
                          , output_info
                          , output_buffer.data());

        std::string text = "HELLO\nWORLD!!!";
        auto text_height = output_info.frame_size.height / 25;
/*
        opencv::text_format_t text_format(opencv::font_t::simplex
                                          , 1.0
                                          , false
                                          , 0xFF000000
                                          , 2
                                          , opencv::v_align_t::center
                                          , opencv::h_align_t::center);


        auto text_size = text_format.text_size(text);
        auto scale = double(text_height) / double(text_size.height);
        text_size.height += text_format.thickness + 1;

        text_size.width *= scale;
        text_size.height *= scale;


        opencv::frame_rect_t rect((output_info.frame_size.width - text_size.width) / 2
                                  , (output_info.frame_size.height - text_size.height) / 2
                                  , text_size.width
                                  , text_size.height);

        std::vector<std::uint8_t> tmp_buffer(text_size.size() * 4);*/



   /*     magick::draw_text(text
                          , output_info.frame_size.width / 2
                          , output_info.frame_size.height / 2
                          , 0xff000000
                          , output_buffer.data()
                          , output_info.frame_size.width
                          , output_info.frame_size.height);

        opencv::draw_image(output_buffer.data()
                           , { output_info.frame_size.width, output_info.frame_size.height }
                           , rect
                           , tmp_buffer.data()
                           , { 0, 0 }
                           , text_size);

        opencv::draw_text(text
                          , tmp_buffer.data()
                          , text_size
                          , text_format
                          , { text_size.width / 2, text_size.height / 2 }
                          , text_height);

        opencv::draw_image(tmp_buffer.data()
                           , text_size
                           , { { 0, 0 }, text_size }
                           , output_buffer.data()
                           , rect.offset
                           , { output_info.frame_size.width, output_info.frame_size.height }
                           , 0.5);*/

/*
        qt::text_format_t text_format(qt::font_t("Times"
                                                 , text_height
                                                 , 10
                                                 , false)
                                      , 0xFF00007F);

        qt::draw_format_t draw_format(0x00FF007F
                                      , 1
                                      , 0x0000FF7F);*/

        /*qt::draw_text(text
                      , text_format
                      , { output_info.frame_size.width / 2, output_info.frame_size.height / 2 }
                      , output_buffer.data()
                      , { output_info.frame_size.width, output_info.frame_size.height }
                      , qt::pixel_format_t::rgba32);*/



        /*
        auto tsz = text_format.font.text_size(text);

        qt::draw_rect(draw_format
                      , { (output_info.frame_size.width - tsz.width) / 2
                          , (output_info.frame_size.height - tsz.height) / 2
                          , tsz.width
                          , tsz.height}
                      , output_buffer.data()
                      , { output_info.frame_size.width, output_info.frame_size.height });

        qt::draw_text(text
                      , text_format
                      , { 0, 0, output_info.frame_size.width, output_info.frame_size.height }
                      , qt::h_align_t::center
                      , qt::v_align_t::center
                      , output_buffer.data()
                      , { output_info.frame_size.width, output_info.frame_size.height }
                      , qt::pixel_format_t::rgba32);*/



        /*
        qt::draw_text(text
                          , output_info.frame_size.width / 2
                          , output_info.frame_size.height / 2
                          , 0xff00007F
                          , text_height
                          , output_buffer.data()
                          , output_info.frame_size.width
                          , output_info.frame_size.height);*/

        //auto flt_delay = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - tp_text).count();


        // auto tmp_buffer = output_buffer;

/*
        opencv::draw_text(text
                          , tmp_buffer.data()
                          , { output_info.frame_size.width, output_info.frame_size.height }
                          , text_format
                          , { output_info.frame_size.width / 2, output_info.frame_size.height / 2 }
                          , text_height);


        opencv::draw_image(tmp_buffer.data()
                           , output_buffer.data()
                           , { output_info.frame_size.width, output_info.frame_size.height }
                           , 0.3);

*/

        /*opencv::draw_text("Hello World!!!"
                          , output_buffer.data()
                          , { output_info.frame_size.width, output_info.frame_size.height }
                          , 0.05);*/


        convert_delay2 = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - tp).count();


        filter_delay += (flt_delay - filter_delay) * delay_factor;
        delay1 += (convert_delay1 - delay1) * delay_factor;
        delay2 += (convert_delay2 - delay2) * delay_factor;

        last_output_buffer = std::move(output_buffer);
        last_image = QImage(last_output_buffer.data(), q_size.width(), q_size.height(), QImage::Format_RGBA8888);

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
    auto& device = rtsp_capturer; //v4l2_capturer;

    if (device->is_opened())
    {
        device->close();
    }
    else
    {
        // std::string uri = "/home/user/h264.avi";
        std::string uri = "/home/user/test_file.mp4";
        // std::string uri = "rtsp://admin:Algont12345678@10.11.4.151";
        // std::string uri = "/home/user/ivcscodec/loading.gif";
        // std::string uri = "v4l2://dev/video0";

        device->open(uri);

        /*
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        auto formats = device->get_supported_formats();

        ui->cbResoulution->clear();

        for (const auto& f : formats)
        {
            core::media::video::video_format_t video_format(core::media::utils::format_conversion::from_v4l2_format(f.pixel_format)
                                                            , { f.size.width, f.size.height }
                                                            , f.fps);


            ui->cbResoulution->addItem(QString::fromStdString(video_format.to_string()));
        }

        auto fmt = device->get_format();

        core::media::video::video_format_t current_video_format(core::media::utils::format_conversion::from_v4l2_format(fmt.pixel_format)
                                                        , { fmt.size.width, fmt.size.height }
                                                        , fmt.fps);


        // device->get_format().

        ui->cbResoulution->setCurrentText(QString::fromStdString(current_video_format.to_string()));

        ui->cbControlList->clear();

        auto controls = device->get_control_list();
        for (const auto& c : controls)
        {

            auto item_string = QString("%1").arg(QString::fromStdString(c.name));

            ui->cbControlList->addItem(item_string);
        }
        */
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


    converter.convert_frames( brg_info
                      , buffer2.data()
                      , yuv_info
                      , yuv_input_buffer.data());

    converter.convert_frames( input_fragment_info
                      , yuv_input_buffer.data()
                      , output_fragment_info
                      , buffer1.data());

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
    painter.drawText(rect(), QString("%1/%2/%3/%4:%5")
                     .arg(QString::number(dec_delay / 1000, 'f', 2))
                     .arg(QString::number(delay1 / 1000, 'f', 2))
                     .arg(QString::number(delay2 / 1000, 'f', 2))
                     .arg(QString::number(fps))
                     .arg(QString::number(real_fps)));

    painter.drawText(0, 40, QString("%1").arg(QString::number(filter_delay / 1000, 'f', 2)));

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

void video_form::test1()
{
    using namespace core::media::video;

    frame_size_t frame_size = { 1280, 720 };

    video_format_t v_format_1(pixel_format_t::yuv420p
                              , frame_size);
    video_format_t v_format_2(pixel_format_t::yuv422p
                              , frame_size);



    auto planes_1 = v_format_1.plane_sizes();
    auto planes_2 = v_format_2.plane_sizes();

    auto frame_size_1 = v_format_1.frame_size();
    auto frame_size_2 = v_format_2.frame_size();

    ffmpeg::frame_size_t f_frame_size( frame_size.width, frame_size.height );
    ffmpeg::video_info_t v_info_1(f_frame_size
                                  , 0
                                  , ffmpeg::pixel_format_yuv420p);
    ffmpeg::video_info_t v_info_2(f_frame_size
                                  , 0
                                  , ffmpeg::pixel_format_yuv422p);

    auto f_planes_1 = v_info_1.plane_sizes();
    auto f_planes_2 = v_info_2.plane_sizes();

    return;
}

void video_form::on_cbAspectRatio_currentIndexChanged(int index)
{
    aspect_ratio_method = static_cast<core::media::video::aspect_ratio_mode_t>(index);
}
