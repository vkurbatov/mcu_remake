#ifndef MEDIA_FRAME_BUFFERED_H
#define MEDIA_FRAME_BUFFERED_H

#include "media_frame.h"
#include "media/common/media_buffer.h"

namespace core
{

namespace media
{

class media_frame_buffered : public virtual i_media_frame
{
    media_buffer    m_media_buffer;
    media_frame     m_media_frame;

    struct media_buffer create_media_buffer(const media_format_t& media_format
                                            , const void** slices = nullptr
                                            , std::size_t size = 0 /* for encoded frame*/);

public:
    media_frame_buffered(const media_format_t& media_format
                         , media::media_buffer&& media_buffer);

    bool swap_buffer(media::media_buffer&& media_buffer);
    media::media_buffer release();

    // i_media_frame interface
public:
    const media_format_t &media_format() const override;
    media_plane_list_t planes() const override;
    std::size_t size() const override;
    bool is_valid() const override;
};

}

}

#endif // MEDIA_FRAME_BUFFERED_H
