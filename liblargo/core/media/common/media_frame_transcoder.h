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
    ffmpeg::libav_transcoder        m_libav_transcoder;

public:
    media_frame_transcoder();
    void reset();

    // i_media_frame_transcoder interface
public:
    bool transcode(const i_media_frame &input_frame
                   , i_media_frame &output_frame) override;

    media_frame_ptr_t transcode(const i_media_frame &input_frame
                                , media_format_t &output_format) override;
};

}

}

#endif // MEDIA_FRAME_TRANSCODER_H
