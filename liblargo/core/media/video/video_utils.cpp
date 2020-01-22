#include "video_utils.h"
#include "video_format.h"
#include "media/common/ffmpeg/libav_base.h"
#include "media/common/utils/format_converter.h"

#include <functional>
#include <algorithm>

#define SWAP(l, r) { auto __tmp = (l); (l) = (r); (r) = __tmp; }

namespace core
{

namespace media
{

namespace video
{

namespace video_utils
{

typedef std::function<void(i_media_plane& plane
                      , const frame_size_t& plane_size)> plane_processor_handle_t;


template<typename T>
static void reverse(T* data
                    , std::size_t size)
{
    auto begin = data;
    auto end = begin + size - 1;
    while (begin < end)
    {
        SWAP(*begin, *end);

        begin++;
        end--;
    }
}

template<typename T>
static void swap_lines(T* lline
                      , T* rline
                      , std::size_t size)
{
    while (size-- > 0)
    {
        SWAP(*lline, *rline);

        lline++;
        rline++;
    }
}

static bool frame_processor(const i_media_frame &media_frame
                     , plane_processor_handle_t process_handle)
{
    const auto& video_format = static_cast<const video_format_t&>(media_frame.media_format());

    if (video_format.media_type == media_type_t::video)
    {
        for (const auto& plane : media_frame.planes())
        {
            process_handle(*plane
                           , video_format.plane_size(plane->plane_id()));
        }

        return true;
    }

    return false;

}

static void vertical_flip_plane(i_media_plane& plane
                                , const frame_size_t& plane_size)
{
    auto plane_data = static_cast<std::uint8_t*>(plane.data());

    for (auto y = 0; y < plane_size.height / 2; y++)
    {
        swap_lines(&plane_data[y * plane_size.width]
                    , &plane_data[(plane_size.height - y - 1) * plane_size.width]
                    , plane_size.width);
    }
}

static void horizontal_flip_plane(i_media_plane& plane
                                  , const frame_size_t& plane_size)
{
    auto plane_data = static_cast<std::uint8_t*>(plane.data());
    for (auto y = 0; y < plane_size.height; y++)
    {
        reverse(plane_data
                , plane_size.width);
        plane_data += plane_size.width;
    }
}

static void fast_rotate_plane(i_media_plane& plane
                                   , const frame_size_t& plane_size)
{
    return reverse(static_cast<std::uint8_t*>(plane.data())
                   , plane_size.size());
}

bool fill_slices(const i_media_frame &media_frame, void *slices[])
{
    auto fill_handle = [&slices](i_media_plane& plane
                                 , const frame_size_t&)
    {
        slices[plane.plane_id()] = plane.data();
    };

    return frame_processor(media_frame
                           , fill_handle);
}


bool blackout(const i_media_frame &media_frame)
{
    void* slices[ffmpeg::max_planes] = {};

    if (fill_slices(media_frame
                    , slices))
    {
        const video_format_t& video_format = static_cast<const video_format_t&>(media_frame.media_format());
        return ffmpeg::video_info_t::blackout(media::utils::format_conversion::to_ffmpeg_format(video_format.pixel_format)
                                       , { video_format.size.width, video_format.size.height }
                                       , slices);
    }

    return false;
}

bool vertical_flip(const i_media_frame &media_frame)
{
    return frame_processor(media_frame
                           , vertical_flip_plane);
}

bool horizontal_flip(const i_media_frame &media_frame)
{
    return frame_processor(media_frame
                           , horizontal_flip_plane);
}

bool fast_rotate(const i_media_frame &media_frame)
{
    return frame_processor(media_frame
                           , fast_rotate_plane);
}


}

}

}

}
