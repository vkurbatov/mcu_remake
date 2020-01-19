#ifndef LIBAV_FILTER_H
#define LIBAV_FILTER_H

#include "libav_base.h"
#include <memory>

namespace ffmpeg
{

struct libav_filter_context_t;
struct libav_filter_context_deleter_t { void operator()(libav_filter_context_t* libav_filter_context_ptr); };

typedef std::unique_ptr<libav_filter_context_t, libav_filter_context_deleter_t> libav_filter_context_ptr_t;

class libav_filter
{
    libav_filter_context_ptr_t      m_filter_context;
    std::string                     m_filter_string;
public:
    libav_filter(const std::string& filter_string);
    bool setup(const std::string& filter_string);
    bool filter(void *data
                , pixel_format_t  pixel_format
                , const frame_size_t& frame_size);
};

}


#endif // LIBAV_FILTER_H
