#include "media_frame_transcoder.h"

#include "media/video/video_frame.h"

namespace core
{

namespace media
{

static bool check_formats(const media_format_t& input_format
                          , const media_format_t& output_format)
{
    if (input_format.media_type == output_format.media_type
            && input_format.is_encoded() != output_format.is_encoded())
    {
        return true;
    }

    return false;
}

static bool check_and_initialize_video(ffmpeg::libav_transcoder& transcoder
                                       , const video::video_format_t& input_format
                                       , video::video_format_t& output_format)
{

}


static bool check_and_initialize(ffmpeg::libav_transcoder& transcoder
                                 , const media_format_t& input_format
                                 , media_format_t& output_format)
{
    if (check_formats(input_format
                      , output_format))
    {

    }

    return false;
}

media_frame_transcoder::media_frame_transcoder()
{

}

bool media_frame_transcoder::transcode(const i_media_frame &input_frame
                                       , i_media_frame &output_frame)
{

}

media_frame_ptr_t media_frame_transcoder::transcode(const i_media_frame &input_frame
                                                    , media_format_t &output_format)
{

}

}

}
