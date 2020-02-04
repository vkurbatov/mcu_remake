#ifndef LIBAV_UTILS_H
#define LIBAV_UTILS_H

#include "libav_base.h"

struct AVCodecContext;
struct AVCodecParameters;

namespace ffmpeg
{

namespace utils
{

bool is_global_header_format(const std::string& format_name);

media_data_t extract_global_header(const stream_info_t& stream_info);

device_type_t fetch_device_type(const std::string& uri);

void merge_codec_params(AVCodecContext& av_context
                            , codec_params_t& codec_params);

}

AVCodecContext& operator << (AVCodecContext& av_context
                             , const media_info_t& media_info);
AVCodecParameters& operator << (AVCodecParameters& av_codecpar
                                , const media_info_t& media_info);

AVCodecContext& operator >> (const media_info_t& media_info
                             , AVCodecContext& av_context);
AVCodecParameters& operator >> (const media_info_t& media_info
                                , AVCodecParameters& av_codecpar);

media_info_t& operator << (media_info_t& media_info
                             , const AVCodecContext& av_context);
media_info_t& operator << (media_info_t& media_info
                                , const AVCodecParameters& av_codecpar);

media_info_t& operator >> (const AVCodecContext& av_context
                           , media_info_t& media_info);
media_info_t& operator >> (const AVCodecParameters& av_codecpar
                           , media_info_t& media_info);



}

#endif // LIBAV_UTILS_H
