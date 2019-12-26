#ifndef ffmpeg_LIBAV_DECODER_H
#define ffmpeg_LIBAV_DECODER_H

#include "libav_base.h"

namespace ffmpeg
{

struct libav_decoder_context_t;
struct libav_decoder_context_deleter_t { void operator()(libav_decoder_context_t* libav_decoder_context_ptr); };

typedef std::unique_ptr<libav_decoder_context_t, libav_decoder_context_deleter_t> libav_decoder_context_ptr_t;

struct decoded_frame_t
{
    stream_info_t   info;
    media_data_t    media_data;
};

typedef std::queue<decoded_frame_t> decoded_queue_t;

class libav_decoder
{
    libav_decoder_context_ptr_t     m_decoder_context;

public:
    libav_decoder();

    bool open(codec_id_t codec_id
              , const media_info_t& media_params = media_info_t()
              , const void* extra_data = nullptr
              , std::size_t extra_data_size = 0);

    bool open(const std::string& decoder_name
              , const media_info_t& media_params = media_info_t()
              , const void* extra_data = nullptr
              , std::size_t extra_data_size = 0);

    bool close();

    bool is_open() const;

    decoded_queue_t decode(const void* data
                           , std::size_t size);

};

}

#endif // ffmpeg_LIBAV_DECODER_H
