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
    media_format_t          m_media_format;
    media_buffer_ptr_t      m_media_buffer;
    frame_id_t              m_frame_id;
    timestamp_t             m_timestamp;
    frame_attributes_t      m_attributes;

public:
    static bool check_media_buffer(const media_format_t& media_format
                                   , const i_media_buffer& media_buffer);

    static timestamp_t now_timestamp();

    static media_frame_ptr_t create(const media_format_t& media_format
                                    , media_buffer_ptr_t media_buffer
                                    , frame_id_t frame_id = 0
                                    , timestamp_t timestamp = 0);

    static media_frame_ptr_t create(const media_format_t& media_format
                                    , media_data_t&& media_data
                                    , frame_id_t frame_id = 0
                                    , timestamp_t timestamp = 0);

public:
    media_frame(const media_format_t& media_format
                , media_buffer_ptr_t media_buffer
                , frame_id_t frame_id = 0
                , timestamp_t timestamp = 0);

    media_frame(const media_format_t& media_format
                , media_data_t&& media_data
                , frame_id_t frame_id = 0
                , timestamp_t timestamp = 0);

    virtual ~media_frame();

    // i_media_frame interface
public:
    const media_format_t &media_format() const override;
    media_format_t &media_format() override;
    media_plane_list_t planes() const override;
    const void* data(std::int32_t offset = 0) const override;
    std::size_t size() const override;
    bool is_valid() const override;
    frame_id_t frame_id() const override;
    timestamp_t timestamp() const override;
    frame_attributes_t frame_attributes() const override;

    void set_frame_id(frame_id_t frame_id) override;
    void set_timestamp(timestamp_t timestamp) override;
    void set_attributes(frame_attributes_t attribute) override;
    void swap(media_buffer_ptr_t &&media_buffer) override;
    media_buffer_ptr_t release() override;

    void clear() override;

};

}

}

#endif // MEDIA_FRAME_H
