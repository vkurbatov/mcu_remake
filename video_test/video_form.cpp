#include "video_form.h"
#include "ui_video_form.h"

#include <QPainter>
#include <QPixmap>
#include <QVideoFrame>
#include "yuv_video_buffer.h"
#include "rgb_video_buffer.h"
#include "core/media/common/ffmpeg/libav_converter.h"

#include <cstring>

ffmpeg_wrapper::libav_converter converter;
std::vector<std::uint8_t>   image_buffer;


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
                dst[j * 3 + 0] = src[j * 4 + 2];
                dst[j * 3 + 1] = src[j * 4 + 1];
                dst[j * 3 + 2] = src[j * 4 + 0];

            }
        }
    }
    ui->setupUi(this);
}

video_form::~video_form()
{
    delete ui;    
}

void video_form::on_pushButton_clicked()
{

    QSize size_src(1280, 720);
    QSize size_dst(1280, 720);

    std::vector<std::uint8_t> buffer1(size_dst.width() * size_dst.height() * 3);

    // buffer1.resize(buffer1.size() * 2);

    auto buffer2 = image_buffer;

    std::memset(buffer1.data(), 200, buffer1.size());
    //std::memset(buffer2.data(), 50, buffer2.size());

    const auto s_x = 360;
    const auto s_y = 240;

    const auto d_x = 360;
    const auto d_y = 240;

    ffmpeg_wrapper::video_info_t    input_frame_info(size_src.width() - s_x * 2, size_src.height() - s_y * 2);
    ffmpeg_wrapper::video_info_t    output_frame_info(size_dst.width() - d_x * 2, size_dst.height() - d_y * 2);

    ffmpeg_wrapper::frame_rect_t    input_frame_rect(s_x, s_y, size_src.width(), size_src.height() - s_y * 2);
    ffmpeg_wrapper::frame_rect_t    output_frame_rect(d_x, d_y, size_dst.width(), size_dst.height() - d_y * 2);

    converter.convert(input_frame_info
                      , input_frame_rect
                      , buffer2.data()
                      , output_frame_info
                      , output_frame_rect
                      , buffer1.data());

    QVideoFrame frame(new rgb_video_buffer(size_dst, buffer1.data()), size_dst, QVideoFrame::PixelFormat::Format_RGB24);

    m_surface.present(frame);
    // close();
}

void video_form::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    painter.setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::FlatCap));
    painter.setBrush(QBrush(Qt::red, Qt::SolidPattern));

    m_surface.paint(&painter);

}
