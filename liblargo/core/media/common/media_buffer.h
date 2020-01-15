#ifndef MEDIA_BUFFER_H
#define MEDIA_BUFFER_H

#include "i_media_buffer.h"

namespace core
{

namespace media
{

class media_buffer : virtual public i_swapped_media_buffer
{
    media_data_t    m_media_data;
    plane_sizes_t   m_planar_sizes;

private:
    media_buffer(const void* data = nullptr
                 , std::size_t size = 0);

    media_buffer(const void * const slices[]
                 , const plane_sizes_t& plane_sizes);

    media_buffer(media_data_t&& media_data
                 , const plane_sizes_t& plane_sizes);

public:
    static media_buffer_ptr_t create(const void* data = nullptr
            , std::size_t size = 0);

    static media_buffer_ptr_t create(const void * const slices[]
                                     , const plane_sizes_t& plane_sizes);

    static media_buffer_ptr_t create(media_data_t&& media_data
                                     , const plane_sizes_t& plane_sizes);

    bool swap(media_buffer&& other_media_buffer);
    bool swap(media_data_t&& media_data
              , plane_sizes_t&& plane_sizes) override;
    media_data_t release() override;


    // i_media_buffer interface
public:
    const void *data(int32_t offset = 0) const override;
    void *data(int32_t offset = 0) override;
    const plane_sizes_t& plane_sizes() const override;
};

}

}

#endif // MEDIA_BUFFER_H
