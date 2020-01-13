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
    const media_format_t&   m_media_format;
    i_media_buffer&         m_media_buffer;

public:    
    media_frame(const media_format_t& media_format
                , i_media_buffer& media_buffer);

    // i_media_frame interface
public:
    const media_format_t &media_format() const override;
    media_plane_list_t planes() const override;
    std::size_t size() const override;
    bool is_valid() const;
};

}

}

#endif // MEDIA_FRAME_H
