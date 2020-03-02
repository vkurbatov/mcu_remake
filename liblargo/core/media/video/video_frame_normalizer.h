#ifndef VIDEO_FRAME_NORMALIZER_H
#define VIDEO_FRAME_NORMALIZER_H

#include "media/common/i_media_frame_normalizer.h"

namespace core
{

namespace media
{

namespace video
{


struct normalizer_context_t;
typedef std::shared_ptr<normalizer_context_t>   normalizer_context_ptr_t;

class video_frame_normalizer : public i_media_frame_normalizer
{
    normalizer_context_ptr_t        m_normalizer_context;
public:
    video_frame_normalizer(const media_format_t& output_format);

    const media_format_t& format() const;
    bool set_format(const media_format_t& format);

    // i_media_frame_normalizer interface
public:
    media_frame_ptr_t normalize(media_frame_ptr_t input_frame) override;
    void reset() override;
};

}

}

}


#endif // VIDEO_FRAME_NORMALIZER_H
