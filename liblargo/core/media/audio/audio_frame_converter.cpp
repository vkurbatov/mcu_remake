#include "audio_frame_converter.h"
#include "audio_resampler.h"
#include "media/common/media_buffer.h"
#include "audio_frame.h"

namespace core
{

namespace media
{

namespace audio
{

audio_frame_converter::audio_frame_converter()
{

}

bool audio_frame_converter::convert(const i_media_frame &input_frame
                                    , i_media_frame &output_frame)
{
    const auto& input_format = input_frame.media_format();
    const auto& output_format = output_frame.media_format();

    if (input_format.media_type == media_type_t::audio
            && output_format.media_type == media_type_t::audio
            && input_format.is_convertable()
            && output_format.is_convertable())
    {
        audio_buffer_t output_buffer;

        if (AudioResampler::Resampling(input_format.audio_info()
                                   , output_format.audio_info()
                                   , input_frame.data()
                                   , input_frame.size()
                                   , output_buffer) > 0)
        {
            output_frame.swap(media_buffer::create(std::move(output_buffer)));
            return true;
        }
    }

    return false;
}

media_frame_ptr_t audio_frame_converter::convert(const i_media_frame &input_frame,
                                                 media_format_t &output_format)
{
    const auto& input_format = input_frame.media_format();

    if (input_format.media_type == media_type_t::audio
            && output_format.media_type == media_type_t::audio
            && input_format.is_convertable()
            && output_format.is_convertable())
    {
        auto output_frame = audio_frame::create(output_format
                                         , media_buffer_t());

        if (convert(input_frame
                    , *output_frame))
        {
            return output_frame;
        }
    }

    return nullptr;
}

}

}

}
