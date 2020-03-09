#include "video_layer_figure.h"
#include "media/common/qt/qt_base.h"


namespace core
{

namespace media
{

namespace video
{

namespace filters
{

static frame_rect_t fetch_rect_from_polylines(const polyline_list_t &polylines)
{
    frame_rect_t frame_rect;

    auto i = 0;

    for (const auto& point : polylines)
    {
        if (i == 0)
        {
            frame_rect.point = point;
        }
        else
        {
            frame_rect.put(point);
        }
        i++;
    }

    return frame_rect;
}

static qt::polylines_t correct_polylines(const polyline_list_t &polylines
                                        , const frame_point_t& point)
{

    auto offset = fetch_rect_from_polylines(polylines).point;

    offset -= point;

    qt::polylines_t corrected_polylines;

    for (const auto& pt : polylines)
    {
        corrected_polylines.emplace_back(pt.x - offset.x
                                         , pt.y - offset.y);
    }

    return std::move(corrected_polylines);
}


static bool draw_polygon(const qt::draw_format_t& draw_format
                         , const qt::polylines_t &polylines
                         , void *pixel_data
                         , const frame_size_t &frame_size)
{
    qt::draw_polygon(draw_format
                     , polylines
                     , pixel_data
                     , { frame_size.width, frame_size.height });
}

static bool draw_rectangles(const qt::draw_format_t& draw_format
                         , const qt::polylines_t &polylines
                         , void *pixel_data
                         , const frame_size_t &frame_size)
{

    for (int i = 0; i < polylines.size(); i++)
    {
        if (i % 2 == 1)
        {
            qt::draw_rect(draw_format
                          , { polylines[i - 1].x
                              , polylines[i - 1].y
                              , polylines[i].x - polylines[i - 1].x
                              , polylines[i].y - polylines[i - 1].y}
                          , pixel_data
                          , { frame_size.width, frame_size.height });
        }
    }
}

static bool draw_ellipses(const qt::draw_format_t& draw_format
                          , const qt::polylines_t &polylines
                          , void *pixel_data
                          , const frame_size_t &frame_size)
{

    for (int i = 0; i < polylines.size(); i++)
    {
        if (i % 2 == 1)
        {
            qt::draw_ellipse(draw_format
                             , { polylines[i - 1].x
                                 , polylines[i - 1].y
                                 , polylines[i].x - polylines[i - 1].x
                                 , polylines[i].y - polylines[i - 1].y}
                             , pixel_data
                             , { frame_size.width, frame_size.height });
        }
    }
}

static decltype(&draw_ellipses) draw_routine(figure_type_t figure_type)
{
    switch(figure_type)
    {
        case figure_type_t::polygon:
            return draw_polygon;
        break;
        case figure_type_t::rectangle:
            return draw_rectangles;
        break;
        case figure_type_t::ellipse:
            return draw_ellipses;
        break;
    }

    return draw_polygon;
}

static bool draw_figure(const figure_format_t &figure_format
                         , const polyline_list_t &polylines
                         , void *pixel_data
                         , const frame_rect_t &target_rect)
{
    qt::draw_format_t draw_format(figure_format.line_color
                                 , figure_format.line_width
                                 , figure_format.background_color);

    auto work_polylines = correct_polylines(polylines
                                            , target_rect.point);
    return draw_routine(figure_format.figure_type)(draw_format, work_polylines, pixel_data, target_rect.size);
}
//-----------------------------------------------------------------------------------
figure_format_t::figure_format_t(figure_type_t figure_type
                                 , uint32_t line_color
                                 , uint32_t background_color
                                 , int32_t line_width)
    : figure_type(figure_type)
    , line_color(line_color)
    , background_color(background_color)
    , line_width(line_width)
{

}
//-----------------------------------------------------------------------------------
video_layer_figure::video_layer_figure(const figure_format_t &figure_format
                                       , const polyline_list_t &polylines)
    : m_figure_format(figure_format)
    , m_polylines(polylines)
{

}

void video_layer_figure::set_format(const figure_format_t &figure_format)
{
    m_figure_format = figure_format;
}

const figure_format_t &video_layer_figure::get_format() const
{
    return m_figure_format;
}

void video_layer_figure::set_polylines(const polyline_list_t &polylines)
{
    m_polylines = polylines;
}

const polyline_list_t &video_layer_figure::get_polylines() const
{
    return m_polylines;
}

bool video_layer_figure::draw_layer(void *pixel_data
                                    , const frame_rect_t &target_rect)
{
    return draw_figure(m_figure_format
                       , m_polylines
                       , pixel_data
                       , target_rect);
}

frame_rect_t video_layer_figure::rect(const frame_size_t &frame_size) const
{
    return fetch_rect_from_polylines(m_polylines);
}

}

}

}

}
