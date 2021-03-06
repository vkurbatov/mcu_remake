#ifndef MEDIA_FRAME_TRANSCODER_H
#define MEDIA_FRAME_TRANSCODER_H

#include "i_media_frame_transcoder.h"
#include "media/common/ffmpeg/libav_transcoder.h"

namespace core
{

namespace media
{

class media_frame_transcoder : virtual public i_media_frame_transcoder
{
    ffmpeg::libav_transcoder            m_libav_transcoder;
    media_format_t                      m_transcoding_format;

public:
    media_frame_transcoder(const media_format_t& transcoding_format);

    const media_format_t& format() const;

    void reset() override;
    bool setup(const media_format_t& transcoding_format) override;


    // i_media_frame_transcoder interface
public:
    bool transcode(const i_media_frame &input_frame, media_frame_queue_t &frame_queue) override;
};

}

}

#endif // MEDIA_FRAME_TRANSCODER_H
