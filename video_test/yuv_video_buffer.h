#ifndef YUV_VIDEO_BUFFER_H
#define YUV_VIDEO_BUFFER_H

#include <QtMultimedia/QAbstractPlanarVideoBuffer>
#include <QSize>

class yuv_video_buffer : public QAbstractPlanarVideoBuffer
{
    MapMode                     m_map_mode;
    QSize                       m_size;
    std::vector<std::uint8_t>   m_data;

public:
    yuv_video_buffer(const QSize& size, const void* data = nullptr);

    // QAbstractVideoBuffer interface
public:
    MapMode mapMode() const override;
    void unmap() override;

    // QAbstractPlanarVideoBuffer interface
public:
    int map(MapMode mode
            , int *numBytes
            , int bytesPerLine[]
            , uchar *data[]) override;



};

#endif // YUV_VIDEO_BUFFER_H
