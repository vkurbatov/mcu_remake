#include "libav_stream_publisher.h"

#include <thread>
#include <mutex>
#include <map>
#include <atomic>
#include <chrono>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavutil/opt.h>
}

#define WBS_MODULE_NAME "ff:publisher"
#include <core-tools/logging.h>

namespace ffmpeg
{

struct libav_format_context_t
{


    libav_format_context_t()
    {

    }

    ~libav_format_context_t()
    {

    }
};

struct libav_stream_publisher_context_t
{

    std::unique_ptr<libav_format_context_t> m_format_context;

    libav_stream_publisher_context_t()
    {

    }

    bool open(const std::string& uri
              , const stream_info_list_t& stream_list);

    bool close();
    bool is_opened() const;
    bool is_established() const;

    stream_info_list_t streams() const;

    bool push_frame(std::int32_t stream_id
                    , const void* data
                    , std::size_t size);
};
//--------------------------------------------------------------------------
void libav_stream_publisher_context_deleter_t::operator()(libav_stream_publisher_context_t *libav_stream_publisher_context_ptr)
{
    delete libav_stream_publisher_context_ptr;
}
//--------------------------------------------------------------------------
libav_stream_publisher::libav_stream_publisher()
 : m_libav_stream_publisher_context()
{

}

bool libav_stream_publisher::open(const std::string &uri, const stream_info_list_t &stream_list)
{

}

bool libav_stream_publisher::close()
{

}

bool libav_stream_publisher::is_opened() const
{

}

bool libav_stream_publisher::is_established() const
{

}

stream_info_list_t libav_stream_publisher::streams() const
{

}

bool libav_stream_publisher::push_frame(int32_t stream_id, const void *data, std::size_t size)
{

}

}
