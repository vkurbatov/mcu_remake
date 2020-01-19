#include "libav_filter.h"

extern "C"
{
#include <libavfilter/avfilter.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
}

namespace ffmpeg
{

const std::string input_buffer_name = "in";
const std::string output_buffer_name = "out";

struct libav_buffers_context
{
    AVFilterGraph		*filter_graph;
    AVFilterContext		*buffersrc_ctx;
    AVFilterContext		*buffersink_ctx;
    bool                is_init;

    libav_buffers_context(const std::string& filter_string)
        : filter_graph(nullptr)
        , buffersrc_ctx(nullptr)
        , buffersink_ctx(nullptr)
        , is_init(false)
    {
        init(filter_string);
    }

    bool init(const std::string& filter_string)
    {
        do
        {
            std::string total_desc = "["
                    + input_buffer_name
                    + "]"
                    + filter_string;

            const AVFilter *buffersrc  = avfilter_get_by_name("buffer");
            const AVFilter *buffersink = avfilter_get_by_name("buffersink");

            if (buffersrc == nullptr || buffersink == nullptr)
            {
                break;
            }

            filter_graph = avfilter_graph_alloc();

            if (filter_graph == nullptr)
            {
                break;
            }

            if (avfilter_graph_create_filter(&buffersrc_ctx
                                             , buffersrc
                                             , input_buffer_name.c_str()
                                             , "video_size=1280x720:pix_fmt=0:time_base=1/1:pixel_aspect=1/1"
                                             , nullptr
                                             , filter_graph) < 0)
            {
                break;
            }

            if (avfilter_graph_create_filter(&buffersink_ctx
                                             , buffersink
                                             , output_buffer_name.c_str()
                                             , nullptr
                                             , nullptr
                                             , filter_graph) < 0)
            {
                break;
            }

            enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };

            if (av_opt_set_int_list(buffersink_ctx
                                    , "pix_fmts"
                                    , pix_fmts
                                    , AV_PIX_FMT_NONE
                                    , AV_OPT_SEARCH_CHILDREN) < 0)
            {
                break;
            }

            AVFilterInOut* inputs = avfilter_inout_alloc();
            AVFilterInOut* outputs = avfilter_inout_alloc();

            inputs->name = const_cast<char*>(input_buffer_name.c_str());
            inputs->filter_ctx = buffersink_ctx;
            inputs->next = nullptr;
            inputs->pad_idx = 0;

            outputs->name = const_cast<char*>(output_buffer_name.c_str());
            outputs->filter_ctx = buffersrc_ctx;
            outputs->next = nullptr;
            outputs->pad_idx = 0;

            auto p_inputs = &inputs;
            auto p_output = &outputs;


            if (avfilter_graph_parse_ptr(filter_graph
                                         , total_desc.c_str()
                                         , &inputs
                                         , &outputs
                                         , nullptr) < 0)
            {
                buffersink_ctx = nullptr;
                buffersrc_ctx = nullptr;
                break;
            }

            is_init = true;
        }
        while(false);

        return is_init;
    }

    bool filter(void *data
                , pixel_format_t pixel_format
                , const frame_size_t &frame_size)
    {

       if (is_init)
       {

           AVFrame* picture = av_frame_alloc();

           picture->width = frame_size.width;
           picture->height = frame_size.height;
           picture->format = pixel_format;

           av_image_fill_arrays(picture->data
                                , picture->linesize
                                , static_cast<std::uint8_t*>(data)
                                , static_cast<AVPixelFormat>(picture->format)
                                , picture->width
                                , picture->height
                                , 1);

           if (av_buffersrc_add_frame_flags(buffersrc_ctx
                                            , picture
                                            , AV_BUFFERSRC_FLAG_KEEP_REF) >= 0)
           {
               return av_buffersink_get_frame(buffersink_ctx
                                              , picture) >= 0;
           }

           //av_buffersrc_add_frame_flags();
       }

       return false;
    }

    void reset()
    {
        is_init = false;

        if (buffersrc_ctx != nullptr)
        {
            avfilter_free(buffersrc_ctx);
            buffersrc_ctx = nullptr;
        }
        if (buffersink_ctx != nullptr)
        {
            avfilter_free(buffersink_ctx);
            buffersink_ctx = nullptr;
        }
        if (filter_graph != nullptr)
        {
            avfilter_graph_free(&filter_graph);
            filter_graph = nullptr;
        }
    }

    ~libav_buffers_context()
    {
        reset();
    }
};

struct libav_filter_context_t
{
    std::string                             m_filter_string;
    std::unique_ptr<libav_buffers_context>  m_buffer_context;

    libav_filter_context_t(const std::string &filter_string)
        : m_filter_string(filter_string)
        , m_buffer_context(new libav_buffers_context(filter_string))
    {

    }

    bool filter(void *data
                , pixel_format_t pixel_format
                , const frame_size_t &frame_size)
    {
        return m_buffer_context->filter(data
                                        , pixel_format
                                        , frame_size);
    }

    bool setup(const std::string &filter_string)
    {
        return false;
    }
};
//--------------------------------------------------------------------------
void libav_filter_context_deleter_t::operator()(libav_filter_context_t *libav_filter_context_ptr)
{
    delete libav_filter_context_ptr;
}
//--------------------------------------------------------------------------
libav_filter::libav_filter(const std::string &filter_string)
    : m_filter_context(new libav_filter_context_t(filter_string))
{

}

bool libav_filter::setup(const std::string &filter_string)
{
    return m_filter_context->setup(filter_string);
}

bool libav_filter::filter(void *data
                          , pixel_format_t pixel_format
                          , const frame_size_t &frame_size)
{
    return m_filter_context->filter(data
                                    , pixel_format
                                    , frame_size);
}

}
