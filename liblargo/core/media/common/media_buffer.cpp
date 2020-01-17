#include "media_buffer.h"

#include <numeric>
#include <cstring>

namespace core
{

namespace media
{

media_buffer::media_buffer(const void *data
                           , std::size_t size)
    : media_buffer(&data
                   , plane_sizes_t({ size }))
{

}

media_buffer::media_buffer(const void * const slices[]
                           , const plane_sizes_t &planar_sizes)
    : m_media_data(std::accumulate(planar_sizes.begin()
                                   , planar_sizes.end()
                                   , 0)
                   , 0)
{
    if (slices != nullptr)
    {
        std::size_t offset = 0;

        for (const auto& sz : planar_sizes)
        {
            if (*slices != nullptr)
            {
                std::memcpy(m_media_data.data() + offset
                            , *slices
                            , sz);
            }

            slices++;
            offset += sz;
        }
    }
}

media_buffer::media_buffer(media_data_t &&media_data)
    : m_media_data(std::move(media_data))
{

}

media_buffer_ptr_t media_buffer::create(const void *data
                                        , std::size_t size)
{
    return media_buffer_ptr_t(new media_buffer(data
                                                , size
                                               )
                              );
}

media_buffer_ptr_t media_buffer::create(const void * const slices[]
                                        , const plane_sizes_t &plane_sizes)
{
    return media_buffer_ptr_t(new media_buffer(slices
                                                , plane_sizes
                                               )
                              );
}

media_buffer_ptr_t media_buffer::create(media_data_t &&media_data)
{
    return media_buffer_ptr_t(new media_buffer(std::move(media_data)));
}

void media_buffer::swap(media_buffer &&other_media_buffer)
{
    swap(std::move(other_media_buffer.m_media_data));
}

void media_buffer::swap(media_data_t &&media_data)
{
    m_media_data.swap(media_data);
}

media_data_t media_buffer::release()
{
    return std::move(m_media_data);
}

void *media_buffer::data(int32_t offset)
{
    return m_media_data.data() + offset;
}

std::size_t media_buffer::size() const
{
    return m_media_data.size();
}

const void *media_buffer::data(int32_t offset) const
{
    return m_media_data.data() + offset;
}

}

}
