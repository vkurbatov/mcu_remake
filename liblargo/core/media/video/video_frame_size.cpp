#include "video_frame_size.h"
namespace core
{

namespace media
{

namespace video
{

template class frame_size_base_t<std::int32_t>;
template class frame_size_base_t<double>;


template<typename T>
frame_size_base_t<T>::frame_size_base_t(T width
                           , T height)
    : width(width)
    , height(height)
{

}
template<typename T>
bool frame_size_base_t<T>::operator==(const frame_size_base_t<T> &frame_size) const
{
    return width == frame_size.width
            && height == frame_size.height;
}
template<typename T>
bool frame_size_base_t<T>::operator !=(const frame_size_base_t<T> &frame_size) const
{
    return ! operator ==(frame_size);
}
template<typename T>
frame_size_base_t<T> &frame_size_base_t<T>::operator +=(const frame_size_base_t<T> &frame_size)
{
    width += frame_size.width;
    height += frame_size.height;
    return *this;
}
template<typename T>
frame_size_base_t<T> &frame_size_base_t<T>::operator -=(const frame_size_base_t<T> &frame_size)
{
    width -= frame_size.width;
    height -= frame_size.height;
    return *this;
}
template<typename T>
bool frame_size_base_t<T>::is_empty() const
{
    return width == 0
            || height == 0;
}
template<typename T>
std::size_t frame_size_base_t<T>::size() const
{
    return width * height;
}

}

}

}
