#include "video_frame_rect.h"

namespace core
{

namespace media
{

namespace video
{

template class frame_rect_base_t<std::int32_t>;
template class frame_rect_base_t<double>;

template<typename T>
frame_rect_base_t<T>::frame_rect_base_t(const frame_point_base_t<T> &point
                           , const frame_size_base_t<T> &size)
    : point(point)
    , size(size)
{

}

template<typename T>
frame_rect_base_t<T>::frame_rect_base_t(const frame_point_base_t<T> &point
                           , const frame_point_base_t<T> &br_point)
    : point(point)
    , size(br_point.x - point.x
           , br_point.y - point.y)
{

}

template<typename T>
frame_rect_base_t<T>::frame_rect_base_t(const frame_size_base_t<T> &size)
    : point(0, 0)
    , size(size)
{

}

template<typename T>
frame_rect_base_t<T>::frame_rect_base_t(T x
                                       , T y
                                       , T width
                                       , T height)
    : point( { x, y } )
    , size( { width, height } )
{

}

template<typename T>
bool frame_rect_base_t<T>::operator ==(const frame_rect_base_t<T> &frame_rect)
{
    return point == frame_rect.point
            && size == frame_rect.size;

}

template<typename T>
bool frame_rect_base_t<T>::operator !=(const frame_rect_base_t<T> &frame_rect)
{
    return !operator ==(frame_rect);
}

template<typename T>
frame_rect_base_t<T>& frame_rect_base_t<T>::operator +=(const frame_point_base_t<T> &frame_point)
{
    point += frame_point;
    return *this;
}

template<typename T>
frame_rect_base_t<T>& frame_rect_base_t<T>::operator -=(const frame_point_base_t<T> &frame_point)
{
    point -= frame_point;
    return *this;
}

template<typename T>
frame_rect_base_t<T>& frame_rect_base_t<T>::operator +=(const frame_size_base_t<T> &frame_size)
{
    size += frame_size;
    return *this;
}

template<typename T>
frame_rect_base_t<T>& frame_rect_base_t<T>::operator -=(const frame_size_base_t<T> &frame_size)
{
    size -= frame_size;
    return *this;
}

template<typename T>
frame_point_base_t<T> frame_rect_base_t<T>::br_point() const
{
    return frame_point_base_t<T>(point.x + size.width, point.y + size.height);
}

template<typename T>
frame_rect_base_t<T> &frame_rect_base_t<T>::set_br_point(const frame_point_base_t<T> &frame_point)
{
    auto br = br_point().merge_max(frame_point);

    size.width = br.x - point.x;
    size.height = br.y - point.y;

    return *this;
}

template<typename T>
bool frame_rect_base_t<T>::is_join(const frame_size_base_t<T> &size) const
{
    return point.x >= 0
            && point.y >= 0
            && br_point().x <= size.width
            && br_point().y <= size.height;
}

template<typename T>
frame_rect_base_t<T>& frame_rect_base_t<T>::merge(const frame_rect_base_t<T> &frame_rect)
{
    point.merge_min(frame_rect.point);

    return set_br_point(br_point().merge_max(frame_rect.br_point()));
}

template<typename T>
frame_rect_base_t<T>& frame_rect_base_t<T>::cut(const frame_rect_base_t<T> &frame_rect)
{
    point.merge_max(frame_rect.point);

    return set_br_point(br_point().merge_min(frame_rect.br_point()));
}

template<typename T>
frame_rect_base_t<T> &frame_rect_base_t<T>::put(const frame_point_base_t<T> &frame_point)
{
    point.merge_min(frame_point);

    return set_br_point(br_point().merge_max(frame_point));
}

}

}

}
