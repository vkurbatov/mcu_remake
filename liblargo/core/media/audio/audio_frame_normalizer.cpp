#include "audio_frame_normalizer.h"
#include "media/common/media_frame_transcoder.h"
#include "media/audio/audio_frame_converter.h"
#include "media/audio/audio_format.h"

namespace core
{

namespace media
{

namespace audio
{

struct normalizer_context_t
{
    std::unique_ptr<media_frame_transcoder> decoder;
    std::unique_ptr<media_frame_transcoder> encoder;
    std::unique_ptr<audio_frame_converter>  converter;
    media_format_t                          input_format;
    media_format_t                          converter_format;
    media_format_t                          output_format;
    bool                                    is_init;

    normalizer_context_t(const media_format_t& output_format)
        : output_format(output_format)
        , is_init(false)
    {

    }

    bool set_output_format(const media_format_t& output_format)
    {
        reset();
        this->output_format = output_format;
    }

    void reset()
    {
        decoder.reset();
        encoder.reset();
        converter.reset();
        is_init = false;
    }

    bool initialize(const media_format_t& input_format)
    {
        if (input_format.media_type == media_type_t::audio)
        {
            reset();

            auto conv_input_format = input_format;
            auto conv_output_format = output_format;

            if (conv_output_format.audio_info().sample_rate == 0)
            {
                conv_output_format.audio_info().sample_rate = input_format.audio_info().sample_rate;
            }

            if (conv_output_format.audio_info().channels == 0)
            {
                conv_output_format.audio_info().channels = input_format.audio_info().channels;
            }

            if (conv_output_format.audio_info().sample_format == audio::sample_format_t::unknown)
            {
                conv_output_format.audio_info().sample_format = input_format.audio_info().sample_format;
            }

            if (input_format == conv_output_format)
            {
                this->input_format = input_format;
                return true;
            }


            if (input_format.is_encoded())
            {
                decoder.reset(new media_frame_transcoder(input_format));
            }

            if (output_format.is_encoded())
            {
                encoder.reset(new media_frame_transcoder(conv_output_format));
            }

            conv_input_format.audio_info().sample_format = conv_input_format.audio_info().raw_sample_format();

            conv_output_format.audio_info().sample_format = conv_output_format.audio_info().raw_sample_format();

            if (conv_input_format != conv_output_format)
            {
                converter_format = conv_output_format;
                converter.reset(new audio_frame_converter());
            }

            this->input_format = input_format;

            return true;
        }

        return false;
    }

    bool check_and_initialize(const media_format_t& input_format)
    {
        if (!is_init || input_format != this->input_format)
        {
            is_init = initialize(input_format);
        }

        return is_init;
    }

    media_frame_ptr_t normalize(media_frame_ptr_t media_frame)
    {
        media_frame_ptr_t out_frame = media_frame;

        if (check_and_initialize(out_frame->media_format()))
        {
            if (decoder != nullptr)
            {
                media_frame_queue_t decoded_frames;

                if (!decoder->transcode(*out_frame
                                        , decoded_frames)
                        || decoded_frames.empty())
                {
                    return nullptr;
                }

                out_frame = std::move(decoded_frames.back());
            }

            if (converter != nullptr)
            {
                out_frame = converter->convert(*out_frame
                                               , converter_format);

                if (out_frame == nullptr)
                {
                    return nullptr;
                }
            }

            if (encoder != nullptr)
            {
                media_frame_queue_t encoded_frames;

                if (!encoder->transcode(*out_frame
                                        , encoded_frames)
                        || encoded_frames.empty())
                {
                    return nullptr;
                }

                out_frame = std::move(encoded_frames.back());
            }

            return out_frame;
        }
    }
};

audio_frame_normalizer::audio_frame_normalizer(const media_format_t &output_format)
    : m_normalizer_context(new normalizer_context_t(output_format))
{

}

const media_format_t &audio_frame_normalizer::format() const
{
    return m_normalizer_context->output_format;
}

bool audio_frame_normalizer::set_format(const media_format_t &format)
{
    return m_normalizer_context->set_output_format(format);
}

media_frame_ptr_t audio_frame_normalizer::normalize(media_frame_ptr_t input_frame)
{
    return m_normalizer_context->normalize(input_frame);
}

void audio_frame_normalizer::reset()
{
    m_normalizer_context->reset();
}

}

}

}
