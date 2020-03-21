#include "simple_media_source.h"

namespace core
{

namespace media
{

simple_media_source::simple_media_source(std::size_t max_queue_size)
    : m_max_queue_size(max_queue_size)
{

}

bool simple_media_source::on_frame(media_frame_ptr_t frame)
{
    if (frame != nullptr)
    {
        std::lock_guard<std::mutex> lg(m_mutex);

        auto stream_id = frame->media_format().stream_id;
        auto it = m_frames.find(stream_id);

        if (it == m_frames.end())
        {
            m_frames.emplace(stream_id
                             , media_frame_queue_t{});

            it = m_frames.find(stream_id);

        }

        auto& frame_queue = it->second;
        frame_queue.push(frame);

        while (frame_queue.size() > m_max_queue_size)
        {
            frame_queue.pop();
        }

        return true;
    }

    return false;
}

media_frame_ptr_t simple_media_source::fetch_frame(stream_id_t stream_id)
{
    std::lock_guard<std::mutex> lg(m_mutex);

    auto it = m_frames.find(stream_id);

    if (it != m_frames.end()
            && !it->second.empty())
    {
        auto frame = it->second.front();
        it->second.pop();
        return frame;
    }

    return nullptr;
}


}

}
