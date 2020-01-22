#ifndef FFMPEG_libav_transcoder_H
#define FFMPEG_libav_transcoder_H

#include "libav_base.h"

namespace ffmpeg
{

struct libav_transcoder_context_t;
struct libav_transcoder_context_deleter_t { void operator()(libav_transcoder_context_t* libav_transcoder_context_ptr); };

typedef std::unique_ptr<libav_transcoder_context_t, libav_transcoder_context_deleter_t> libav_transcoder_context_ptr_t;


typedef frame_t decoded_frame_t;
typedef frame_queue_t decoded_queue_t;
/*struct decoded_frame_t
{
    stream_info_t   info;
    media_data_t    media_data;
};*/

//typedef std::queue<decoded_frame_t> decoded_queue_t;

class libav_transcoder
{
    libav_transcoder_context_ptr_t     m_transcoder_context;

public:
    libav_transcoder();

    bool open(const stream_info_t& steam_info);

    bool close();

    bool is_open() const;    

    decoded_queue_t decode(const void* data
                           , std::size_t size);


    codec_id_t codec_id() const;

};

}

#endif // ffmpeg_libav_transcoder_H
