#ifndef MEDIA_BUFFER_H
#define MEDIA_BUFFER_H

#include "i_media_buffer.h"

namespace core
{

namespace media
{

class media_buffer : virtual public i_media_buffer
{
    media_data_t    m_media_data;
    planar_sizes_t  m_planar_sizes;

public:
    media_buffer(const void* data = nullptr
                 , std::size_t size = 0);

    media_buffer(const void** slices
                 , const planar_sizes_t& planar_sizes);

    media_buffer(media_data_t&& media_data
                 , const planar_sizes_t& planar_sizes);

    bool swap(media_buffer&& other_media_buffer);
    bool swap(media_data_t&& media_data
              , planar_sizes_t&& planar_sizes);
    media_data_t release();


    // i_media_buffer interface
public:
    const void *data(int32_t offset = 0) const override;
    void *data(int32_t offset = 0) override;
    const planar_sizes_t& planar_sizes() const override;
};

}

}

#endif // MEDIA_BUFFER_H
