#include "video_layout_manager_custom.h"
#include <cmath>

namespace core
{

namespace media
{

namespace video
{

const layout_id_t max_layout_id = 17;

static relative_layout_list_t create_auto_layout_list(layout_id_t layout_id)
{
    if (layout_id < 2)
    {
        return { { 0.0, 0.0, 1.0, 1.0 } };
    }

    relative_layout_list_t layout_list;

    auto col_count = static_cast<std::int32_t>(sqrt(layout_id) + 0.999);
    auto last_col_count = layout_id % col_count;
    auto row_count = static_cast<std::int32_t>(layout_id) / col_count + static_cast<std::int32_t>(last_col_count != 0);

    double l_width = 1.0 / col_count;
    double l_heigth = 1.0 / row_count;

    for (auto row = 0; row < row_count; row ++)
    {
        auto l_y = static_cast<double>(row) * l_heigth;

        auto x_offset = 0.0;

        if ((row + 1 == row_count) && (last_col_count != 0))
        {
            x_offset = (static_cast<double>(col_count - last_col_count) * l_width) / 2.0;
        }

        for (auto col = 0; col < col_count && layout_list.size() < layout_id; col++)
        {
            auto l_x = x_offset + static_cast<double>(col) * l_width;
            layout_list.emplace_back(relative_frame_point_t( l_x, l_y )
                                     , relative_frame_size_t(l_width, l_heigth)
                                     );
        }
    }

    return layout_list;
}

static layouts_list_t create_auto_layouts(layout_id_t max_layout_id)
{

    layouts_list_t layouts_list;

    for (layout_id_t layout_id = 0; layout_id < max_layout_id; layout_id++)
    {
        layouts_list.emplace_back(std::move(create_auto_layout_list(layout_id)));
    }

    return std::move(layouts_list);
}


layouts_list_t auto_layouts = std::move(create_auto_layouts(max_layout_id));

video_layout_manager_custom::video_layout_manager_custom(const layouts_list_t& custom_layouts)
    : m_custom_layouts(custom_layouts)
{

}

void video_layout_manager_custom::set_layouts(const layouts_list_t &custom_layouts)
{
    m_custom_layouts = custom_layouts;
}

const layouts_list_t &video_layout_manager_custom::layouts() const
{
    return m_custom_layouts;
}

bool video_layout_manager_custom::fetch_layout(layout_id_t layout_id
                                                  , stream_order_t order
                                                  , relative_frame_rect_t &layout)
{
    layout_id = layout_id > max_layout_id
            ? max_layout_id
            : layout_id;

    const auto& current_layout = layout_id < m_custom_layouts.size()
                        ? m_custom_layouts
                        : auto_layouts;

    if (layout_id < current_layout.size()
            && order < current_layout[layout_id].size())
    {
        layout = current_layout[layout_id][order];
        return true;
    }

    return false;
}

}

}

}
