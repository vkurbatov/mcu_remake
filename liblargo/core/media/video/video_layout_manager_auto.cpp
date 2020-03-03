#include "video_layout_manager_auto.h"
#include <cmath>

namespace core
{

namespace media
{

namespace video
{

const layout_id_t max_layout_id = 20;

static relative_layout_list_t create_layout_list(layout_id_t layout_id)
{
    if (layout_id < 2)
    {
        return { relative_frame_rect_t( relative_frame_point_t(0.0, 0.0)
                                            , relative_frame_size_t(1.0, 1.0)) };
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

video_layout_manager_auto::video_layout_manager_auto()
{

}

bool video_layout_manager_auto::fetch_layout(layout_id_t layout_id
                                             , stream_order_t order
                                             , relative_frame_rect_t &layout)
{
    if (layout_id <= max_layout_id
            && order < layout_id)
    {
        auto it = m_layouts.find(layout_id);

        if (it == m_layouts.end())
        {
            m_layouts.emplace(layout_id
                              , std::move(create_layout_list(layout_id)));
        }

        it = m_layouts.find(layout_id);

        layout = it->second[order];

        return true;
    }

    return false;
}

}

}

}
