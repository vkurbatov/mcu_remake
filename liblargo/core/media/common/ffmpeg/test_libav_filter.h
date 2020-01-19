#ifndef TEST_LIBAV_FILTER_H
#define TEST_LIBAV_FILTER_H

#include "libav_base.h"

#include <memory>


namespace ffmpeg
{

struct libav_filter_params_t
{

    enum libav_font_flags_t : std::uint32_t
    {
        ff_none =							1 << 0,
        ff_no_scale =						1 << 1,
        ff_no_hinting =						1 << 2,
        ff_render =							1 << 3,
        ff_no_bitmap =						1 << 4,
        ff_vertical_layout =				1 << 5,
        ff_force_autohint =					1 << 6,
        ff_crop_bitmap =					1 << 7,
        ff_pedantic =						1 << 8,
        ff_ignore_global_advance_width	=   1 << 9,
        ff_no_recurse =						1 << 10,
        ff_ignore_transform	=				1 << 11,
        ff_monochrome =						1 << 12,
        ff_linear_design =					1 << 13,
        ff_no_autohint =					1 << 14,
    };

    std::uint32_t	shadow_color;	// 0xRRGGBB[AA]
    std::string		font_file;		// Path to ttf file
    std::uint32_t	font_color;		// 0xRRGGBB[AA]
    std::uint32_t	font_size;
    std::uint32_t	font_flags;		// Example: font_flags="pedantic+linear_design"
    std::uint32_t	border_width;
    std::uint32_t	border_color;

    std::string get_description() const;

    bool operator == (const libav_filter_params_t& params);
    bool operator != (const libav_filter_params_t& params);
};

struct libav_filter_point_t
{
    double x;
    double y;

    bool operator == (const libav_filter_point_t& point) { return x != point.x && y != point.y; }
    bool operator != (const libav_filter_point_t& point) { return !operator == (point); }
};

const std::uint32_t no_color = 0xFFFFFFFF;

const std::uint32_t default_shadow_color = no_color;
const std::string	default_font_file = "";						// Path to ttf file
const std::uint32_t	default_font_color = no_color;		// 0xRRGGBB[AA]
const std::uint32_t	default_font_size = 16;
const std::uint32_t	default_font_flags = libav_filter_params_t::ff_pedantic | libav_filter_params_t::ff_linear_design; // Example: font_flags="pedantic+linear_design"
const std::uint32_t	default_border_width = 0;
const std::uint32_t	default_border_color = no_color;

const libav_filter_params_t default_libav_filter_params =
{
    default_shadow_color,
    default_font_file,
    default_font_color,
    default_font_size,
    default_font_flags,
    default_border_width,
    default_border_color
};

struct test_libav_filter_context_t;

struct libav_filter_context_deleter_t
{
    void operator()(test_libav_filter_context_t* context);
};

using libav_filter_handle_t = std::unique_ptr<test_libav_filter_context_t, libav_filter_context_deleter_t>;

class test_libav_filter
{

    libav_filter_handle_t	m_handle;
    libav_filter_params_t	m_libav_params;
    libav_filter_point_t	m_position;
    std::string				m_text;

    bool					m_need_init;
    std::uint32_t			m_width;
    std::uint32_t			m_height;

public:
    test_libav_filter(const libav_filter_params_t& params = default_libav_filter_params
            , const std::string& text = ""
            , const libav_filter_point_t& position = { 0.0, 0.0 });

    void set_text(const std::string& text);
    const std::string& get_text() const;

    void set_position(const libav_filter_point_t& position);
    const libav_filter_point_t& get_position();

    void set_params(const libav_filter_params_t& params);
    const libav_filter_params_t& get_params();

    // i_frame_filter interface
public:
    bool operator ()(void *data
                     , pixel_format_t  pixel_format
                     , const frame_size_t& frame_size);
};

}

#endif
