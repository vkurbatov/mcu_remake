#ifndef VIDEO_FILTER_FLIP_H
#define VIDEO_FILTER_FLIP_H

#include "video_filter.h"

namespace core
{

namespace media
{

namespace video
{

namespace filters
{

enum class flip_method_t
{
    none = 0,
    vertical = 1,
    horizontal = 2,
    both = flip_method_t::vertical | flip_method_t::horizontal
};

class video_filter_flip : public video_filter
{
    flip_method_t             m_flip_method;

public:
    video_filter_flip(flip_method_t flip_method = flip_method_t::none);

    flip_method_t flip_method() const;
    void set_flip_method(flip_method_t flip_method);

    // video_filter interface
protected:
    bool internal_filter(i_video_frame &video_frame) const override;
};

}

}

}

}


#endif // VIDEO_FILTER_ROTATE_H
