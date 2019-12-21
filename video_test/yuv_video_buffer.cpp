#include "yuv_video_buffer.h"

#include <cstring>

yuv_video_buffer::yuv_video_buffer(const QSize& size, const void* data)
    : QAbstractPlanarVideoBuffer(HandleType::NoHandle)
    , m_map_mode(MapMode::NotMapped)
    , m_size(size)
    , m_data((m_size.width() * m_size.height() * 12) / 8, 0)
{
    if (data != nullptr)
    {
        std::memcpy(m_data.data(), data, m_data.size());
    }
}

QAbstractVideoBuffer::MapMode yuv_video_buffer::mapMode() const
{
    return m_map_mode;
}

void yuv_video_buffer::unmap()
{
    m_map_mode = MapMode::NotMapped;
}

int yuv_video_buffer::map(QAbstractVideoBuffer::MapMode mode
                          , int *numBytes
                          , int bytesPerLine[]
                          , uchar *data[])
{
    m_map_mode = mode;

    *numBytes = m_data.size();

    bytesPerLine[0] = m_size.width();
    bytesPerLine[1] = m_size.width() / 2;
    bytesPerLine[2] = m_size.width() / 2;

    data[0] = m_data.data();
    data[1] = data[0] + m_size.width() * m_size.height();
    data[2] = data[1] + (m_size.width() * m_size.height()) / 4;

    return 3;

}

