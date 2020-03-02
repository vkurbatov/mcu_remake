#ifndef AUDIO_FRAME_CONVERTER_H
#define AUDIO_FRAME_CONVERTER_H

#include "media/common/i_media_frame_converter.h"

namespace core
{

namespace media
{

namespace audio
{

class audio_frame_converter : virtual public i_media_frame_converter
{
public:
    audio_frame_converter();
    // i_media_frame_converter interface
public:
    bool convert(const i_media_frame &input_frame, i_media_frame &output_frame) override;
    media_frame_ptr_t convert(const i_media_frame &input_frame, media_format_t &output_format) override;
};

}

}

}

#endif // AUDIO_FRAME_CONVERTER_H
