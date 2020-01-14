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

media_buffer::media_buffer(const void **slices
                           , const plane_sizes_t &planar_sizes)
    : m_media_data(std::accumulate(planar_sizes.begin()
                                   , planar_sizes.end()
                                   , 0)
                   , 0)
    , m_planar_sizes(planar_sizes)
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

media_buffer::media_buffer(media_data_t &&media_data
                           , const plane_sizes_t& planar_sizes)
    : m_planar_sizes(planar_sizes)
{
    auto sz = std::accumulate(planar_sizes.begin()
                              , planar_sizes.end()
                              , 0);
    if (media_data.size() != sz)
    {
        m_media_data.resize(sz, 0);
    }
    else
    {
        m_media_data = std::move(media_data);
    }
}

bool media_buffer::swap(media_buffer &&other_media_buffer)
{
    return swap(std::move(other_media_buffer.m_media_data)
                , std::move(other_media_buffer.m_planar_sizes));
}

bool media_buffer::swap(media_data_t &&media_data
                        , plane_sizes_t&& planar_sizes)
{
    if (media_data.size() == std::accumulate(planar_sizes.begin()
                                             , planar_sizes.end()
                                             , 0))
    {
        m_media_data.swap(media_data);
        m_planar_sizes.swap(planar_sizes);
        return true;
    }

    return false;
}

media_data_t media_buffer::release()
{
    m_planar_sizes.clear();
    return std::move(m_media_data);
}

void *media_buffer::data(int32_t offset)
{
    return m_media_data.data() + offset;
}

const void *media_buffer::data(int32_t offset) const
{
    return m_media_data.data() + offset;
}

const plane_sizes_t &media_buffer::plane_sizes() const
{
    return m_planar_sizes;
}

}

}
