#include "video_frame_point.h"
#include <algorithm>

namespace core
{

namespace media
{

namespace video
{

template class frame_point_base_t<std::int32_t>;
template class frame_point_base_t<double>;

template<typename T>
frame_point_base_t<T>::frame_point_base_t(T x
                                       , T y)
    : x(x)
    , y(y)
{

}

template<typename T>
bool frame_point_base_t<T>::operator ==(const frame_point_base_t<T> &frame_point) const
{
    return x == frame_point.x
            && y == frame_point.y;
}

template<typename T>
bool frame_point_base_t<T>::operator !=(const frame_point_base_t<T> &frame_point) const
{
    return ! operator ==(frame_point);
}

template<typename T>
frame_point_base_t<T> &frame_point_base_t<T>::operator +=(const frame_point_base_t<T> &frame_point)
{
    x += frame_point.x;
    y += frame_point.y;
    return *this;
}

template<typename T>
frame_point_base_t<T> &frame_point_base_t<T>::operator -=(const frame_point_base_t<T> &frame_point)
{
    x -= frame_point.x;
    y -= frame_point.y;
    return *this;
}

template<typename T>
bool frame_point_base_t<T>::is_null() const
{
    return x == 0
            && y == 0;
}

template<typename T>
frame_point_base_t<T> &frame_point_base_t<T>::merge_min(const frame_point_base_t<T> &frame_point)
{
    x = std::min(x, frame_point.x);
    y = std::min(y, frame_point.y);
    return *this;
}

template<typename T>
frame_point_base_t<T> &frame_point_base_t<T>::merge_max(const frame_point_base_t<T> &frame_point)
{
    x = std::max(x, frame_point.x);
    y = std::max(y, frame_point.y);
    return *this;
}


}

}

}
