#include "rgb_video_buffer.h"

#include <cstring>
#include <cmath>

rgb_video_buffer::rgb_video_buffer(const QSize &size, const void *data)
    : QAbstractVideoBuffer(HandleType::NoHandle)
    , m_map_mode(MapMode::NotMapped)
    , m_size(size)
    , m_data(size.width() * size.height() * 3, 0)
{
    std::srand(time(nullptr));

    if (data != nullptr)
    {
        std::memcpy(m_data.data(), data, m_data.size());
    }
    else
    {
        for (auto& c : m_data)
        {
            c = rand() % 0x100;
        }
    }
}

rgb_video_buffer::~rgb_video_buffer()
{
    m_map_mode = MapMode::NotMapped;
}

QAbstractVideoBuffer::MapMode rgb_video_buffer::mapMode() const
{
    return m_map_mode;
}

void rgb_video_buffer::unmap()
{
    m_map_mode = MapMode::NotMapped;
}

uchar *rgb_video_buffer::map(QAbstractVideoBuffer::MapMode mode
                             , int *numBytes
                             , int *bytesPerLine)
{
    m_map_mode = mode;

    *numBytes = m_data.size();
    *bytesPerLine = m_size.width() * 3;

    return m_data.data();
}

void rgb_video_buffer::fill(uchar r, uchar g, uchar b)
{
    for (int i = 0; i < m_data.size(); i += 3)
    {
        m_data[i + 0] = r;
        m_data[i + 1] = g;
        m_data[i + 2] = b;
    }
}
