#ifndef MEDIA_FRAME_H
#define MEDIA_FRAME_H

#include "i_media_frame.h"
#include "i_media_buffer.h"

namespace core
{

namespace media
{

class media_frame : virtual public i_media_frame
{
protected:
    media_buffer_ptr_t      m_media_buffer;
    frame_id_t              m_frame_id;
    timestamp_t             m_timestamp;

public:
    static bool check_media_buffer(const media_format_t& media_format
                                   , const i_media_buffer& media_buffer);

public:
    media_frame(media_buffer_ptr_t media_buffer
                , frame_id_t frame_id = 0
                , timestamp_t timestamp = 0);

    media_frame(media_data_t&& media_data
                , frame_id_t frame_id = 0
                , timestamp_t timestamp = 0);

    virtual ~media_frame();

    // i_media_frame interface
public:
    media_plane_list_t planes() const override;
    std::size_t size() const override;
    bool is_valid() const;
    frame_id_t frame_id() const override;
    timestamp_t timestamp() const override;

    void swap(media_buffer_ptr_t &&media_buffer) override;
    media_buffer_ptr_t release() override;

};

}

}

#endif // MEDIA_FRAME_H
