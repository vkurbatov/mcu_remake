#ifndef SIMPLE_MEDIA_SOURCE_H
#define SIMPLE_MEDIA_SOURCE_H

#include "i_media_source.h"
#include "i_media_sink.h"

#include <map>
#include <mutex>

namespace core
{

namespace media
{

const std::size_t default_queue_size = 10;

class simple_media_source : virtual public i_media_source
        , virtual public i_media_sink
{
    typedef std::map<stream_id_t, media_frame_queue_t> media_frame_map_t;

    std::size_t                     m_max_queue_size;
    media_frame_map_t               m_frames;
    std::mutex                      m_mutex;
public:
    simple_media_source(std::size_t max_queue_size = default_queue_size);

    // i_media_sink interface
public:
    bool on_frame(media_frame_ptr_t frame) override;

    // i_media_source interface
public:
    media_frame_ptr_t fetch_frame(stream_id_t stream_id) override;
};

}

}

#endif // SIMPLE_MEDIA_SOURCE_H
