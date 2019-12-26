#ifndef VIDEO_SURFACE_H
#define VIDEO_SURFACE_H

#include <QtMultimedia/QAbstractVideoSurface>

class video_surface : public QAbstractVideoSurface
{
    QWidget*        m_widget;
    QImage          m_last_image;

public:
    video_surface(QWidget *widget, QObject *parent = 0);
    ~video_surface();

    // QAbstractVideoSurface interface
public:
    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const override;
    bool present(const QVideoFrame &frame) override;
 public:
    void paint(QPainter *painter, const QVector<QRect>& rects);
};

#endif // VIDEO_SURFACE_H
