#include "video_surface.h"

#include <QtMultimedia/QVideoSurfaceFormat>
#include <QWidget>
#include <QPainter>

#include <qvideoframe.h>
#include <mutex>

std::mutex image_mutex;

video_surface::video_surface(QWidget *widget, QObject *parent)
    : QAbstractVideoSurface(parent)
    , m_widget(widget)
{
}

video_surface::~video_surface()
{

}

QList<QVideoFrame::PixelFormat> video_surface::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
    if (handleType == QAbstractVideoBuffer::NoHandle)
    {
        return { QVideoFrame::PixelFormat::Format_RGB24, QVideoFrame::PixelFormat::Format_BGR24 };
    }

    return {};
}

bool video_surface::present(const QVideoFrame &frame)
{
    //static bool first = true;
    // if (surfaceFormat().pixelFormat() == frame.pixelFormat())
    {
        QVideoFrame new_frame = frame;

        new_frame.map(QAbstractVideoBuffer::MapMode::ReadOnly);
        m_last_image = std::move(QImage(frame.bits(),
                     frame.width(),
                     frame.height(),
                     frame.bytesPerLine(),
                     QVideoFrame::imageFormatFromPixelFormat(frame.pixelFormat())));

        new_frame.unmap();
        m_widget->repaint(0, 0, frame.width(), frame.height());
        // m_widget->on
    }
}

void video_surface::paint(QPainter *painter, const QVector<QRect>& rects)
{
    // return;
    for (const auto& rect : rects)
    {
        //QRect d_rect = rect;
        // d_rect.moveTop(30);        
        painter->drawImage(rect, m_last_image, rect);
    }

}
