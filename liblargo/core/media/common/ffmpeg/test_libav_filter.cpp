#include "test_libav_filter.h"

extern "C"
{
#include <libavutil/frame.h>
#include <libavutil/pixfmt.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
}

#include <cstdio>
#include <iostream>

//"drawtext=fontfile=/usr/share/fonts/truetype/freefont/FreeSerif.ttf: text='Test Text': x=100: y=50: fontsize=24: fontcolor=yellow@0.2: box=1: boxcolor=red@0.2"

namespace ffmpeg
{

namespace filter_utils
{
    /*template<typename T>
    std::string to_hex_string(const T& value)
    {
        std::string result("");

        auto ptr = reinterpret_cast<const std::uint8_t*>(&value);
        for (auto i = 0; i < sizeof(T); i++)
        {
            char hex_byte[3];
            std::sprintf(hex_byte, "%.2X", *ptr);
            result.append(hex_byte);

            ptr++;
        }
        return std::move(result);
    }*/

    std::string get_hex_color(std::uint32_t color)
    {
        char hex_value[11];
        std::sprintf(hex_value, "0x%.8X", color);

        return hex_value;
    }

    std::string font_flags_to_descr(std::uint32_t flags)
    {
        std::string result("");

        const std::string filter_map[] =
        { "default", "no_scale", "no_hinting", "render", "no_bitmap", "vertical_layout", "force_autohint",
        "crop_bitmap", "pedantic", "ignore_global_advance_width", "no_recurse", "ignore_transform", "monochrome",
        "linear_design", "no_autohint"};

        bool first = true;

        for (std::uint32_t i = 1; i <= 14; i++)
        {
            if ((flags & (1 << i)) != 0)
            {
                if (!first)
                {
                    result.append("+");
                }
                else
                {
                    first = false;
                }
                result.append(filter_map[i]);
            }
        }

        return result;
    }
}

struct test_libav_filter_context_t
{
    AVFilterGraph		*filter_graph;
    AVFilterContext		*buffersrc_ctx;
    AVFilterContext		*buffersink_ctx;
    AVFrame				*frame;
    AVFrame				*filt_frame;

    test_libav_filter_context_t(const std::string& filter_description, std::uint32_t width, std::uint32_t height)
        : filter_graph(nullptr)
        , buffersrc_ctx(nullptr)
        , buffersink_ctx(nullptr)
        , frame(nullptr)
        , filt_frame(nullptr)
    {
        setup(filter_description, width, height);
    }

    ~test_libav_filter_context_t()
    {
        reset();
    }

    bool setup(const std::string& filter_description, std::uint32_t width, std::uint32_t height)
    {
        bool result = false;

        static bool first_init = true;

        if (first_init)
        {
            avfilter_register_all();
            first_init = false;
        }

        const std::string full_description = std::string("[in_v]drawtext=") + filter_description;

        if (!is_init())
        {
            const AVFilter *buffersrc  = avfilter_get_by_name("buffer");
            const AVFilter *buffersink = avfilter_get_by_name("buffersink");

            enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };

            const std::string args = "video_size=" + std::to_string(width) + "x" + std::to_string(height) + ":pix_fmt="
                    + std::to_string(AV_PIX_FMT_YUV420P) + ":time_base=1/1:pixel_aspect=1/1";
            do
            {
                if (buffersrc == nullptr || buffersink == nullptr)
                {
                    break;
                }

                filter_graph = avfilter_graph_alloc();

                if (filter_graph == nullptr)
                {
                    break;
                }

                if (avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in_v", args.c_str(), nullptr, filter_graph) < 0)
                {
                    break;
                }

                if (avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out", nullptr, nullptr, filter_graph) < 0)
                {
                    break;
                }
                if (av_opt_set_int_list(buffersink_ctx, "pix_fmts", pix_fmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN) < 0)
                {
                    break;
                }

                AVFilterInOut *outputs = avfilter_inout_alloc();
                AVFilterInOut *inputs = avfilter_inout_alloc();

                outputs->name       = av_strdup("in_v");
                outputs->filter_ctx = buffersrc_ctx;
                outputs->pad_idx    = 0;

                inputs->name       = av_strdup("out");
                inputs->filter_ctx = buffersink_ctx;
                inputs->pad_idx    = 0;
                inputs->next       = nullptr;


                if (avfilter_graph_parse_ptr(filter_graph, full_description.c_str(), &inputs, &outputs, nullptr) < 0)
                {
                    avfilter_inout_free(&inputs);
                    avfilter_inout_free(&outputs);

                    break;
                }

                avfilter_inout_free(&inputs);
                avfilter_inout_free(&outputs);


                if (avfilter_graph_config(filter_graph, nullptr) < 0)
                {
                    break;
                }

                frame = av_frame_alloc();

                if (frame == nullptr)
                {
                    break;
                }

                filt_frame = av_frame_alloc();

                if (filt_frame == nullptr)
                {
                    break;
                }

                result = true;
            }
            while(false);

        }
        else
        {
            char res[1024] = {};
            result = avfilter_graph_send_command(filter_graph, "drawtext", "reinit", filter_description.c_str(), res, sizeof(res), 0) >= 0;
        }

        if (result == false)
        {
            reset();
        }

        return result;
    }

    bool is_init() const
    {
        return filter_graph != nullptr
                && buffersrc_ctx != nullptr
                && buffersink_ctx != nullptr
                && frame != nullptr
                && filt_frame != nullptr;
    }

    void reset()
    {
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
        if (frame != nullptr)
        {
            av_frame_unref(frame);
            av_frame_free(&frame);
            frame = nullptr;
        }
        if (filt_frame != nullptr)
        {
            av_frame_unref(filt_frame);
            av_frame_free(&filt_frame);
            filt_frame = nullptr;
        }
    }
};

void libav_filter_context_deleter_t::operator()(test_libav_filter_context_t *context)
{
    delete context;
}

// ----------------------------------------------------------------------------------

std::string libav_filter_params_t::get_description() const
{
    std::string result = "";

    if (shadow_color != no_color)
    {
        result.append(":shadowcolor=");
        result.append(filter_utils::get_hex_color(shadow_color));
    }

    if (!font_file.empty())
    {
        result.append(":fontfile=");
        result.append(font_file);
    }

    if (font_flags != 0)
    {
        result.append(":ft_load_flags=");
        result.append(filter_utils::font_flags_to_descr(font_flags));
    }

    if (font_color != no_color)
    {
        result.append(":fontcolor=");
        result.append(filter_utils::get_hex_color(font_color));
    }

    if (font_size > 0)
    {
        result.append(":fontsize=");
        result.append(std::to_string(font_size));
    }

    if (border_width > 0)
    {
        result.append(":box=1");

        if (border_color != no_color)
        {
            result.append(":boxcolor=");
            result.append(filter_utils::get_hex_color(border_color));
        }

        result.append(":boxborderw=");
        result.append(std::to_string(border_width));

    }

    return std::move(result);
}

bool libav_filter_params_t::operator==(const libav_filter_params_t &params)
{
    return memcmp(this, &params, sizeof(params)) == 0;
}

bool libav_filter_params_t::operator !=(const libav_filter_params_t &params)
{
    return !operator ==(params);
}

static std::string create_filter_description(const libav_filter_params_t &params
                                             , const std::string &text
                                             , const libav_filter_point_t &position
                                             , std::uint32_t width
                                             , std::uint32_t height)
{
    std::string result("");

    result.append("text='");
    result.append(text);
    result.append("'");

    result.append(params.get_description());

    auto x = static_cast<std::uint32_t>(static_cast<double>(width) * position.x);
    auto y = static_cast<std::uint32_t>(static_cast<double>(height) * position.y);

    result.append(":x=(");
    result.append(std::to_string(x));
    result.append("-text_w/2)");


    result.append(":y=(");
    result.append(std::to_string(y));
    result.append("-text_h/2)");

    std::cout << result << std::endl;

    return std::move(result);
}

//----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------

test_libav_filter::test_libav_filter(const libav_filter_params_t &params, const std::string &text, const libav_filter_point_t &position)
    : m_libav_params(params)
    , m_text(text)
    , m_position(position)
    , m_need_init(true)
    , m_width(0)
    , m_height(0)
{

}

void test_libav_filter::set_text(const std::string &text)
{
    m_need_init = m_text != text;
    m_text = text;
}

const std::string &test_libav_filter::get_text() const
{
    return m_text;
}

void test_libav_filter::set_position(const libav_filter_point_t &position)
{
    m_need_init = m_position != position;
    m_position = position;
}

const libav_filter_point_t &test_libav_filter::get_position()
{
    return m_position;
}

void test_libav_filter::set_params(const libav_filter_params_t &params)
{
    m_need_init = m_libav_params != params;
    m_libav_params = params;
}

const libav_filter_params_t &test_libav_filter::get_params()
{
    return m_libav_params;
}

bool test_libav_filter::operator ()(void *data
                               , pixel_format_t  pixel_format
                               , const frame_size_t& frame_size)
{
    bool result = false;

    if (!frame_size.is_null())
    {
        if (pixel_format == 0)
        {
            if (frame_size.width != m_width || frame_size.height != m_height)
            {
                m_handle.reset(nullptr);
            }

            m_width = frame_size.width;
            m_height = frame_size.height;

            if (m_handle == nullptr)
            {
                m_handle.reset(
                            new test_libav_filter_context_t(
                                create_filter_description(
                                    m_libav_params
                                    , m_text
                                    , m_position
                                    , m_width
                                    , m_height)
                            , m_width
                            , m_height)
                            );
                m_need_init = false;
            }
            else
            {
                if (m_need_init)
                {
                    m_handle->setup(create_filter_description(
                                        m_libav_params
                                        , m_text
                                        , m_position
                                        , m_width
                                        , m_height)
                                    , m_width
                                    , m_height);
                    m_need_init = false;
                }
            }

            if (m_handle != nullptr && m_handle->is_init())
            {
                test_libav_filter_context_t& context = *m_handle;

                AVPicture* picture = (AVPicture*)context.frame;

                context.frame->width = m_width;
                context.frame->height = m_height;
                context.frame->format = AV_PIX_FMT_YUV420P;

                auto frame = static_cast<std::uint8_t*>(data);

                avpicture_fill(picture, frame, AV_PIX_FMT_YUV420P, m_width, m_height);

                if (av_buffersrc_add_frame_flags(context.buffersrc_ctx, context.frame, AV_BUFFERSRC_FLAG_KEEP_REF) >= 0)
                {
                    do
                    {
                        int ret = av_buffersink_get_frame(context.buffersink_ctx, context.filt_frame);

                        if (ret == AVERROR(EAGAIN))
                        {
                            continue;
                        }
                        else if (ret >= 0)
                        {

                            uint32_t pitch_y = context.filt_frame->linesize[0];
                            uint32_t pitch_u = context.filt_frame->linesize[1];
                            uint32_t pitch_v = context.filt_frame->linesize[2];

                            uint8_t *av_y = context.filt_frame->data[0];
                            uint8_t *av_u = context.filt_frame->data[1];
                            uint8_t *av_v = context.filt_frame->data[2];

                            for (auto i = 0; i < context.filt_frame->height; i++)
                            {
                                for (auto y = 0; y < context.filt_frame->width; y++)
                                {
                                    *frame++ = *(av_y+y);
                                }
                                av_y += pitch_y;
                            }
                            for (auto i = 0; i < context.filt_frame->height / 2; i++)
                            {
                                for (auto y = 0; y < context.filt_frame->width / 2; y++)
                                {
                                    *frame++ = *(av_u + y);
                                }
                                av_u += pitch_u;
                            }
                            for (auto i = 0; i < context.filt_frame->height / 2; i++)
                            {
                                for (auto y = 0; y < context.filt_frame->width / 2; y++)
                                {
                                    *frame++ = *(av_v+y);
                                }
                                av_v += pitch_v;
                            }
                        }
                        else
                        {
                            // LOG
                        }

                        av_frame_unref(context.filt_frame);

                        break;
                    }
                    while(true);
                }

                result = true;
            }

        }
    }

    return result;
}

}
