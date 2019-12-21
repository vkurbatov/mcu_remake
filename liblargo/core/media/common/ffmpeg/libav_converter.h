#ifndef LIBAV_CONVERTER_H
#define LIBAV_CONVERTER_H

#include "libav_base.h"

namespace ffmpeg_wrapper
{

struct libav_converter_context_t;
struct libav_converter_context_deleter_t { void operator()(libav_converter_context_t* libav_converter_context_ptr); };

typedef std::unique_ptr<libav_converter_context_t, libav_converter_context_deleter_t> libav_converter_context_ptr_t;

class libav_converter
{
    libav_converter_context_ptr_t   m_converter_context;

public:
    libav_converter();

    std::size_t convert(const video_info_t& input_frame_info
                        , const frame_rect_t& input_frame_rect
                        , const void* input_frame
                        , const video_info_t& output_frame_info
                        , const frame_rect_t& output_frame_rect
                        , void* output_frame);

    void reset();
};

}

#endif // LIBAV_CONVERTER_H
