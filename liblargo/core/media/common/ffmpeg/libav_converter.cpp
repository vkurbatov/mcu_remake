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

namespace ffmpeg_wrapper
{

const std::size_t max_planes = 4;

namespace utils
{

std::size_t fill_arrays(const video_info_t& frame_info
                        , const frame_rect_t& frame_rect
                        , std::uint8_t* slices[max_planes]
                        , std::int32_t strides[max_planes]
                        , const void* data
                        , std::int32_t align = 0)
{
    std::size_t result = 0;

    auto ret = av_image_fill_arrays(slices
                                    , strides
                                    , static_cast<const std::uint8_t*>(data)
                                    , static_cast<AVPixelFormat>(frame_info.pixel_format)
                                    , frame_info.size.width
                                    , frame_info.size.height
                                    , align);
    if (ret >= 0)
    {
        for (auto i = 0; i < max_planes && strides[i] != 0; i++)
        {
            auto k = static_cast<double>(frame_info.size.width) / static_cast<double>(strides[i]);

            strides[i] = frame_rect.size.width / k;
            slices[i] += strides[i] * frame_rect.point.y + static_cast<std::uint32_t>(frame_rect.point.x / k);
            //slices[i] += strides[i] * static_cast<std::uint32_t>(frame_rect.point.y / k) + static_cast<std::uint32_t>(frame_rect.point.x / k);
        }

        result = ret;
    }

    return result;
}

}

struct libav_converter_context_t
{ 
    struct SwsContext*  m_sws_context;
    video_info_t        m_input_frame_info;
    video_info_t        m_output_frame_info;
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

    bool check_or_create_context(const video_info_t& input_frame_info
                                  , const video_info_t& output_frame_info)
    {
        if (m_sws_context == nullptr
                || m_input_frame_info.pixel_format != input_frame_info.pixel_format
                || m_input_frame_info.size != input_frame_info.size
                || m_output_frame_info.pixel_format != output_frame_info.pixel_format
                || m_output_frame_info.size != output_frame_info.size)
        {
            m_sws_context = sws_getCachedContext(m_sws_context
                                                 , input_frame_info.size.width
                                                 , input_frame_info.size.height
                                                 , static_cast<AVPixelFormat>(input_frame_info.pixel_format)
                                                 , output_frame_info.size.width
                                                 , output_frame_info.size.height
                                                 , static_cast<AVPixelFormat>(output_frame_info.pixel_format)
                                                 , static_cast<std::uint32_t>(m_scaling_method)
                                                 , nullptr
                                                 , nullptr
                                                 , nullptr);

        }

        if (m_sws_context != nullptr)
        {
            m_input_frame_info = input_frame_info;
            m_output_frame_info = output_frame_info;

            return true;
        }

        return false;
    }   

    std::size_t convert(const video_info_t& input_frame_info
                        , const frame_rect_t& input_frame_rect
                        , const void* input_frame
                        , const video_info_t& output_frame_info
                        , const frame_rect_t& output_frame_rect
                        , void* output_frame)
    {
        std::size_t result = 0;

        if (check_or_create_context(input_frame_info, output_frame_info))
        {
            std::uint8_t* src_slice[max_planes] = { };
            std::int32_t src_stride[max_planes] = { };

            std::uint8_t* dst_slice[max_planes] = { };
            std::int32_t dst_stride[max_planes] = { };

            auto sz_input = utils::fill_arrays(input_frame_info
                                               , input_frame_rect
                                               , src_slice
                                               , src_stride
                                               , input_frame
                                               , 1);

            auto sz_output = utils::fill_arrays(output_frame_info
                                                , output_frame_rect
                                                , dst_slice
                                                , dst_stride
                                                , output_frame
                                                , 1);

            if (sz_input != 0
                    && sz_output != 0)
            {
                auto h_corr = input_frame_info.size == output_frame_info.size ? 1 : 0;

                auto sws_result = sws_scale(m_sws_context
                                            , src_slice
                                            , src_stride
                                            , h_corr
                                            , input_frame_info.size.height - h_corr
                                            , dst_slice
                                            , dst_stride);

                if (sws_result > 0)
                {                    
                    std::cout << "src_stride = " << src_stride[0] << ", dst_stride = " << dst_stride[0] << std::endl;
                    result = sws_result;
                }
            }
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

std::size_t libav_converter::convert(const video_info_t& input_frame_info
                                     , const frame_rect_t& input_frame_rect
                                     , const void* input_frame
                                     , const video_info_t& output_frame_info
                                     , const frame_rect_t& output_frame_rect
                                     , void* output_frame)
{
    return m_converter_context->convert(input_frame_info
                                        , input_frame_rect
                                        , input_frame
                                        , output_frame_info
                                        , output_frame_rect
                                        , output_frame);
}

void libav_converter::reset()
{
    m_converter_context->reset();
}


}
