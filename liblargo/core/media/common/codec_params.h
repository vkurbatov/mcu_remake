#ifndef CODEC_PARAMS_H
#define CODEC_PARAMS_H

#include <cstdint>
#include <vector>
#include <string>

namespace core
{

namespace media
{

typedef std::vector<std::uint8_t> extra_data_t;

struct codec_params_t
{
    std::uint32_t   bitrate;
    std::uint32_t   max_bitrate;
    std::uint32_t   gop_size;
    std::uint32_t   frame_size;
    bool            global_header;
    extra_data_t    extra_data;
    std::string     parameters;

    codec_params_t(std::uint32_t bitrate = 0
                   , std::uint32_t max_bitrate = 0
                   , std::uint32_t gop_size = 0
                   , std::size_t frame_size = 0
                   , bool global_header = false
                   , const std::string& parameters = {});

};

}

}
#endif // CODEC_PARAM_H
