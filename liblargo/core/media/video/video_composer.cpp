#include "video_composer.h"


namespace core
{

namespace media
{

namespace video
{

bool video_composer::open(const std::string &uri)
{

}

bool video_composer::close()
{

}

bool video_composer::is_open() const
{

}

bool video_composer::is_established() const
{

}

media_format_list_t video_composer::streams() const
{

}

const control_parameter_list_t &video_composer::controls() const
{

}

bool video_composer::set_control(const std::string &control_name
                                 , const variant &control_value)
{

}

variant video_composer::get_control(const std::string &control_name
                                    , const variant &default_value) const
{

}

bool video_composer::on_frame(const i_media_frame &frame)
{

}

}

}

}
