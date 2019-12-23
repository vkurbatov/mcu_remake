#include "video_surface.h"

#include <QtMultimedia/QVideoSurfaceFormat>
#include <QWidget>
#include <QPainter>

#include <qvideoframe.h>

video_surface::video_surface(QWidget *widget, QObject *parent)
    : QAbstractVideoSurface(parent)
    , m_widget(widget)
{
}

video_surface::~video_surface()
{
    m_last_frame.unmap();
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
    auto s_f = surfaceFormat().pixelFormat();
    auto f_f = frame.pixelFormat();
    // if (surfaceFormat().pixelFormat() == frame.pixelFormat())
    {
        m_last_frame = frame;
        m_widget->repaint(0, 0, m_last_frame.width(), m_last_frame.height());
    }
}

void video_surface::paint(QPainter *painter)
{
    m_last_frame.map(QAbstractVideoBuffer::ReadOnly);

    QImage image(m_last_frame.bits(),
                 m_last_frame.width(),
                 m_last_frame.height(),
                 m_last_frame.bytesPerLine(),
                 QVideoFrame::imageFormatFromPixelFormat(m_last_frame.pixelFormat()));

    painter->drawImage(0, 0, image);

    m_last_frame.unmap();
}
