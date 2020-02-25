#include "codec_params.h"


core::media::codec_params_t::codec_params_t(uint32_t bitrate
                                            , uint32_t max_bitrate
                                            , uint32_t gop_size
                                            , std::size_t frame_size
                                            , bool global_header
                                            , const std::string &parameters)
    : bitrate(bitrate)
    , max_bitrate(max_bitrate)
    , gop_size(gop_size)
    , frame_size(frame_size)
    , global_header(global_header)
    , parameters(parameters)
    , extra_data(0)
{

}
