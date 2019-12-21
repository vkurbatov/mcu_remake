#ifndef RGB_VIDEO_BUFFER_H
#define RGB_VIDEO_BUFFER_H

#include <QtMultimedia/QAbstractPlanarVideoBuffer>
#include <QSize>

class rgb_video_buffer : public QAbstractVideoBuffer
{
    MapMode                     m_map_mode;
    QSize                       m_size;
    std::vector<std::uint8_t>   m_data;

public:
    rgb_video_buffer(const QSize& size, const void* data = nullptr);
    ~rgb_video_buffer();

    // QAbstractVideoBuffer interface
public:
    MapMode mapMode() const override;
    void unmap() override;
    uchar *map(MapMode mode, int *numBytes, int *bytesPerLine) override;

    void fill(uchar r, uchar g, uchar b);
};


#endif // RGB_VIDEO_BUFFER_H
