#ifndef I_MEDIA_FRAME_CONVERTER_H
#define I_MEDIA_FRAME_CONVERTER_H

#include "i_media_frame.h"

namespace core
{

namespace media
{

class i_media_frame_converter
{
public:
    virtual bool convert(const i_media_frame& input_frame
                         , i_media_frame& output_frame) = 0;

    virtual media_frame_ptr_t convert(const i_media_frame& input_frame
                                      , media_format_t& output_format) = 0;
};

}

}

#endif // I_MEDIA_FRAME_CONVERTER_H
