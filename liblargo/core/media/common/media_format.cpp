#include "media_format.h"
#include "media/video/video_format.h"
#include "media/audio/audio_format.h"

namespace core
{

namespace media
{

struct format_info_stub_t : public virtual i_format_info
{
public:
    bool is_encoded() const override { return false; }
    bool is_convertable() const override { return false; }
    bool is_planar() const override { return false; }
    std::size_t frame_size() const override { return 0; }
    std::size_t planes() const override { return 0; }
    plane_sizes_t plane_sizes() const override { return { 0 }; }
    std::string to_string() const override { return ""; }
};

namespace data
{

struct data_info_t : virtual public format_info_stub_t
{

};

}

/*
namespace audio
{

struct audio_info_t : virtual public format_info_stub_t
{

};

}*/

struct info_storage_t
{
    video::video_info_t     video_info;
    audio::audio_info_t     audio_info;
    data::data_info_t       data_info;

    info_storage_t(const video::video_info_t& video_info)
        : video_info(video_info)
        , audio_info{}
        , data_info{}
    {

    }

    info_storage_t(const audio::audio_info_t& audio_info)
        : video_info{}
        , audio_info(audio_info)
        , data_info{}

    {

    }
    info_storage_t(const data::data_info_t& data_info = data::data_info_t())
        : video_info{}
        , audio_info{}
        , data_info(data_info)
    {

    }

    const i_format_info& format_info(media_type_t media_type) const
    {
        switch(media_type)
        {
            case media_type_t::video:
                return video_info;
            break;
            case media_type_t::audio:
                return audio_info;
            break;
            case media_type_t::data:
                return data_info;
            break;
        }
    }
};


media_format_t::media_format_t(media_type_t media_type)
    : info_storage(new info_storage_t())
    , media_type(media_type)
    , stream_id(stream_id)
{

}

media_format_t::media_format_t(const video::video_info_t &video_info
                               , stream_id_t stream_id)
    : info_storage(new info_storage_t(video_info))
    , media_type(media_type_t::video)
    , stream_id(stream_id)

{

}

media_format_t::media_format_t(const audio::audio_info_t &audio_info
                               , stream_id_t stream_id)
    : info_storage(new info_storage_t(audio_info))
    , media_type(media_type_t::audio)
    , stream_id(stream_id)

{

}

media_format_t::media_format_t(const data::data_info_t &data_info
                               , stream_id_t stream_id)
    : info_storage(new info_storage_t(data_info))
    , media_type(media_type_t::data)
    , stream_id(stream_id)

{

}

media_format_t::media_format_t(const media_format_t &media_format)
    : info_storage(new info_storage_t(*media_format.info_storage))
    , media_type(media_format.media_type)
    , stream_id(media_format.stream_id)
    , parameters(media_format.parameters)

{

}

media_format_t &media_format_t::operator=(const media_format_t &media_format)
{
    *info_storage = *media_format.info_storage;
    media_type = media_format.media_type;
    stream_id = media_format.stream_id;
    parameters = media_format.parameters;
}

bool media_format_t::is_encoded() const
{
    return info_storage->format_info(media_type).is_encoded();
}

bool media_format_t::is_convertable() const
{
    return info_storage->format_info(media_type).is_convertable();
}

bool media_format_t::is_planar() const
{
    return info_storage->format_info(media_type).is_planar();
}

std::size_t media_format_t::frame_size() const
{
    return info_storage->format_info(media_type).frame_size();
}

std::size_t media_format_t::planes() const
{
    if (is_encoded())
    {
        return 1;
    }
    else
    {
        return info_storage->format_info(media_type).planes();
    }
}

plane_sizes_t media_format_t::plane_sizes() const
{
    return info_storage->format_info(media_type).plane_sizes();
}

bool media_format_t::is_valid() const
{
    return is_encoded()
            || planes() > 0;
}


video::video_info_t &media_format_t::video_info()
{
    return info_storage->video_info;
}

audio::audio_info_t &media_format_t::audio_info()
{
    return info_storage->audio_info;
}

data::data_info_t &media_format_t::data_info()
{
    return info_storage->data_info;
}

const video::video_info_t &media_format_t::video_info() const
{
    return info_storage->video_info;
}

const audio::audio_info_t &media_format_t::audio_info() const
{
    return info_storage->audio_info;
}

const data::data_info_t &media_format_t::data_info() const
{
    return info_storage->data_info;
}


bool media_format_t::operator ==(const media_format_t &media_format)
{

    return media_type == media_format.media_type
            && extra_data.size() == media_format.extra_data.size();
}

bool media_format_t::operator !=(const media_format_t &media_format)
{
    return !operator == (media_format);
}

std::string media_format_t::to_string() const
{
    return info_storage->format_info(media_type).to_string();
    /*static std::string names[] = { "audio", "video", "data" };
    return names[static_cast<std::uint32_t>(media_type)];*/

}

external_media_info_t::external_media_info_t(void *data
                                             , std::size_t size
                                             , const std::string &specific_options)
    : data(data)
    , size(size)
    , specific_options(specific_options)
{

}

}

}
