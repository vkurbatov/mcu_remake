#include "libav_converter.h"

#include <cstring>

extern "C"
{
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixdesc.h>
}

#include <iostream>

namespace ffmpeg
{

namespace utils
{

void fill_items(const frame_rect_t& frame_rect
               , std::uint8_t* slices[max_planes]
               , std::int32_t strides[max_planes]
               , std::size_t frame_size
               , bool is_flip)
{
    const auto base_slice = *slices;
    auto y_k = 1.0;

    for (auto i = 0; i < max_planes && strides[i] != 0; i++)
    {
        auto x_k = static_cast<double>(frame_rect.size.width) / static_cast<double>(strides[i]);

        auto plane_size = strides[i + 1] == 0 || max_planes == i + 1
                ? frame_size - (slices[i] - base_slice)
                : slices[i + 1] - slices[i];

        if (plane_size != 0)
        {
            y_k = strides[i] * frame_rect.size.height / static_cast<double>(plane_size);
        }

        auto x_offset = static_cast<std::int32_t>(frame_rect.offset.x / x_k);
        auto y_offset = strides[i] * static_cast<std::int32_t>(frame_rect.offset.y / y_k);

        if (is_flip)
        {
            slices[i] += strides[i] * static_cast<std::int32_t>((frame_rect.size.height - 1) / y_k)
            - y_offset + x_offset;

            strides[i] = -strides[i];

        }
        else
        {
            slices[i] += y_offset + x_offset;
        }
    }
}

std::size_t fill_arrays(const frame_rect_t& frame_rect
                        , pixel_format_t pixel_format
                        , std::uint8_t* slices[max_planes]
                        , std::int32_t strides[max_planes]
                        , void * const slice_list[]
                        , bool is_flip = false
                        , std::int32_t align = default_frame_align)
{
    auto frame_size =  video_info_t::frame_size(pixel_format
                                                , frame_rect.size);

    if (frame_size > 0)
    {
        auto i = 0;
        for (const auto& sz : video_info_t::plane_sizes(pixel_format
                                       , frame_rect.size))
        {
            slices[i] = const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(slice_list[i]));
            strides[i] = sz.width;
            i++;
        }

        fill_items(frame_rect
                  , slices
                  , strides
                  , frame_size
                  , is_flip);
    }

    return frame_size;
}

}

struct libav_converter_context_t
{ 
    struct SwsContext*  m_sws_context;
    frame_size_t        m_input_frame_size;
    pixel_format_t      m_input_pixel_format;
    frame_size_t        m_output_frame_size;
    pixel_format_t      m_output_pixel_format;
    scaling_method_t    m_scaling_method;

    libav_converter_context_t(scaling_method_t scaling_method)
        : m_sws_context(nullptr)
        , m_scaling_method(scaling_method)
    {

    }

    ~libav_converter_context_t()
    {
        reset();
    }   

    bool check_or_create_context(const frame_size_t& input_frame_size
                                 , pixel_format_t input_pixel_format
                                 , const frame_size_t& output_frame_size
                                 , pixel_format_t output_pixel_format)
    {
        if (m_sws_context == nullptr
                || m_input_pixel_format != input_pixel_format
                || m_input_frame_size != input_frame_size
                || m_output_pixel_format != output_pixel_format
                || m_output_frame_size != output_frame_size)
        {
            m_sws_context = sws_getCachedContext(m_sws_context
                                                 , input_frame_size.width
                                                 , input_frame_size.height
                                                 , static_cast<AVPixelFormat>(input_pixel_format)
                                                 , output_frame_size.width
                                                 , output_frame_size.height
                                                 , static_cast<AVPixelFormat>(output_pixel_format)
                                                 , static_cast<std::uint32_t>(m_scaling_method)
                                                 , nullptr
                                                 , nullptr
                                                 , nullptr);

            if (m_sws_context != nullptr)
            {
                m_input_pixel_format = input_pixel_format;
                m_input_frame_size = input_frame_size;
                m_output_pixel_format = output_pixel_format;
                m_output_frame_size = output_frame_size;
            }
        }

        return m_sws_context != nullptr;
    }   

    std::size_t scale(const fragment_info_t& input_fragment_info
                      , void* const input_slices[]
                      , const fragment_info_t& output_fragment_info
                      , void* output_slices[]
                      , bool is_flip)
    {
        std::size_t result = 0;

        if (check_or_create_context(input_fragment_info.frame_rect.size
                                                 , input_fragment_info.pixel_format
                                                 , output_fragment_info.frame_rect.size
                                                 , output_fragment_info.pixel_format))
        {
            std::uint8_t* src_slice[max_planes] = { };
            std::int32_t src_stride[max_planes] = { };

            std::uint8_t* dst_slice[max_planes] = { };
            std::int32_t dst_stride[max_planes] = { };

            auto sz_input = utils::fill_arrays({ input_fragment_info.frame_rect.offset, input_fragment_info.frame_size }
                                               , input_fragment_info.pixel_format
                                               , src_slice
                                               , src_stride
                                               , input_slices
                                               , is_flip);

            auto sz_output = utils::fill_arrays({ output_fragment_info.frame_rect.offset, output_fragment_info.frame_size }
                                                , output_fragment_info.pixel_format
                                                , dst_slice
                                                , dst_stride
                                                , output_slices);

            if (sz_input != 0
                    && sz_output != 0)
            {
                auto h_corr = sz_input == sz_output
                        && input_fragment_info.frame_rect.size == output_fragment_info.frame_rect.size
                            ? 1
                            : 0;

                auto sws_result = sws_scale(m_sws_context
                                            , src_slice
                                            , src_stride
                                            , h_corr
                                            , input_fragment_info.frame_rect.size.height - h_corr
                                            , dst_slice
                                            , dst_stride);

                if (sws_result > 0)
                {
                    // std::cout << "src_stride = " << src_stride[0] << ", dst_stride = " << dst_stride[0] << std::endl;
                    result = sz_output;
                }
            }

        }

        return result;
    }

    std::size_t convert_frames(const fragment_info_t& input_fragment_info
                               , const void* input_frame
                               , const fragment_info_t& output_fragment_info
                               , void* output_frame
                               , bool is_flip)
    {

        std::size_t result = 0;

        if (input_fragment_info.is_full()
                && input_fragment_info == output_fragment_info
                && is_flip == false)
        {
            result = input_fragment_info.get_frame_size();
            if (output_frame != input_frame)
            {
                std::memcpy(output_frame
                            , input_frame
                            , result);
            }
        }
        else
        {


            void *input_slices[max_planes] = {};
            void *output_slices[max_planes] = {};

            video_info_t::split_slices(input_fragment_info.pixel_format
                                       , input_fragment_info.frame_size
                                       , input_slices
                                       , input_frame);


            video_info_t::split_slices(output_fragment_info.pixel_format
                                       , output_fragment_info.frame_size
                                       , output_slices
                                       , output_frame);


            result = scale(input_fragment_info
                           , input_slices
                           , output_fragment_info
                           , output_slices
                           , is_flip);


        }

        return result;
    }

    std::size_t convert_slices(const fragment_info_t& input_fragment_info
                               , void* const input_slices[]
                               , const fragment_info_t& output_fragment_info
                               , void* output_slices[]
                               , bool is_flip)
    {
        std::size_t result = 0;

        if (input_fragment_info.is_full()
                && input_fragment_info == output_fragment_info
                && is_flip == false)
        {
            result = input_fragment_info.get_frame_size();

            if (input_slices != output_slices)
            {
                auto i = 0;
                for (const auto& sz : video_info_t::plane_sizes(input_fragment_info.pixel_format
                                                                , input_fragment_info.frame_size))
                {
                    if (input_slices[i] != output_slices[i])
                    {
                        std::memcpy(output_slices[i]
                                    , input_slices[i]
                                    , sz.size());
                    }
                    i++;
                }
            }
        }
        else
        {
            result = scale(input_fragment_info
                           , input_slices
                           , output_fragment_info
                           , output_slices
                           , is_flip);
        }

        return result;
    }

    std::size_t convert_to_slices(const fragment_info_t& input_fragment_info
                                  , const void* input_frame
                                  , const fragment_info_t& output_fragment_info
                                  , void* output_slices[]
                                  , bool is_flip)
    {

        std::size_t result = 0;

        if (input_fragment_info.is_full()
                && input_fragment_info == output_fragment_info
                && is_flip == false)
        {
            result = input_fragment_info.get_frame_size();

            auto i = 0;
            std::size_t offset = 0;
            for (const auto& sz : video_info_t::plane_sizes(input_fragment_info.pixel_format
                                                            , input_fragment_info.frame_size))
            {
                std::memcpy(output_slices[i]
                            , static_cast<const std::uint8_t*>(input_frame) + offset
                            , sz.size());

                offset += sz.size();
                i++;
            }
        }
        else
        {

            void *input_slices[max_planes] = {};

            video_info_t::split_slices(input_fragment_info.pixel_format
                                       , input_fragment_info.frame_size
                                       , input_slices
                                       , input_frame);

            result = scale(input_fragment_info
                           , input_slices
                           , output_fragment_info
                           , output_slices
                           , is_flip);


        }

        return result;
    }

    std::size_t convert_to_frame(const fragment_info_t& input_fragment_info
                                  , void* const input_slices[]
                                  , const fragment_info_t& output_fragment_info
                                  , void* output_frame
                                  , bool is_flip)
    {
        std::size_t result = 0;

        if (input_fragment_info.is_full()
                && input_fragment_info == output_fragment_info
                && is_flip == false)
        {
            result = input_fragment_info.get_frame_size();

            auto i = 0;
            std::size_t offset = 0;
            for (const auto& sz : video_info_t::plane_sizes(input_fragment_info.pixel_format
                                                            , input_fragment_info.frame_size))
            {
                std::memcpy(static_cast<std::uint8_t*>(output_frame) + offset
                            , input_slices[i]
                            , sz.size());

                offset += sz.size();
                i++;
            }
        }
        else
        {
            void *output_slices[max_planes] = {};

            video_info_t::split_slices(output_fragment_info.pixel_format
                                       , output_fragment_info.frame_size
                                       , output_slices
                                       , output_frame);

            result = scale(input_fragment_info
                           , input_slices
                           , output_fragment_info
                           , output_slices
                           , is_flip);
        }

        return result;
    }


    void reset()
    {
        if (m_sws_context != nullptr)
        {
            sws_freeContext(m_sws_context);
            m_sws_context = nullptr;
        }
    }

    void reset(scaling_method_t scaling_method)
    {
        reset();

        m_scaling_method = scaling_method;
    }

};
// -----------------------------------------------------------------------------
void libav_converter_context_deleter_t::operator()(libav_converter_context_t *libav_converter_context_ptr)
{
    delete libav_converter_context_ptr;
}
// -----------------------------------------------------------------------------
libav_converter::libav_converter(scaling_method_t scaling_method)
    : m_converter_context(new libav_converter_context_t(scaling_method))
{

}

std::size_t libav_converter::convert_frames(const fragment_info_t& input_fragment_info
                                           , const void* input_frame
                                           , const fragment_info_t& output_fragment_info
                                           , void* output_frame
                                           , bool is_flip)
{
    return m_converter_context->convert_frames(input_fragment_info
                                               , input_frame
                                               , output_fragment_info
                                               , output_frame
                                               , is_flip);
}

std::size_t libav_converter::convert_slices(const fragment_info_t &input_fragment_info
                                            , void * const input_slices[]
                                            , const fragment_info_t &output_fragment_info
                                            , void *output_slices[]
                                            , bool is_flip)
{
    return m_converter_context->convert_slices(input_fragment_info
                                               , input_slices
                                               , output_fragment_info
                                               , output_slices
                                               , is_flip);
}

std::size_t libav_converter::convert_to_slices(const fragment_info_t &input_fragment_info
                                               , const void *input_frame
                                               , const fragment_info_t &output_fragment_info
                                               , void *output_slices[]
                                               , bool is_flip)
{
    return m_converter_context->convert_to_slices(input_fragment_info
                                                 , input_frame
                                                 , output_fragment_info
                                                 , output_slices
                                                 , is_flip);
}

std::size_t libav_converter::convert_to_frame(const fragment_info_t &input_fragment_info
                                              , void * const input_slices[]
                                              , const fragment_info_t &output_fragment_info
                                              , void *output_frame
                                              , bool is_flip)
{
    return m_converter_context->convert_to_frame(input_fragment_info
                                                 , input_slices
                                                 , output_fragment_info
                                                 , output_frame
                                                 , is_flip);
}

void libav_converter::reset(scaling_method_t scaling_method)
{
    m_converter_context->reset(scaling_method);
}

void libav_converter::reset()
{
    m_converter_context->reset();
}

scaling_method_t libav_converter::scaling_method() const
{
    return m_converter_context->m_scaling_method;
}


}
