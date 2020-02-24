#ifndef MEDIA_FORMAT_H
#define MEDIA_FORMAT_H

#include <cstdint>
#include <vector>
#include <string>
#include <memory>

namespace core
{

namespace media
{

enum class media_type_t
{
    audio,
    video,
    data
};

typedef std::vector<std::size_t> plane_sizes_t;
typedef std::vector<std::uint8_t> extra_data_t;

typedef std::int32_t stream_id_t;
const stream_id_t no_stream = -1;

struct media_format_t;

// typedef std::shared_ptr<media_format_t> media_format_ptr_t;
typedef std::vector<media_format_t> media_format_list_t;

struct info_storage_t;
//struct info_storage_deleter_t { void operator()(info_storage_t* info_storage_ptr); };
typedef std::shared_ptr<info_storage_t> info_storage_ptr_t;

namespace video
{
struct video_info_t;
}

namespace audio
{
struct audio_format_t;
typedef audio_format_t audio_info_t;
}

namespace data
{
struct data_info_t;
}

class i_format_info
{
public:
    virtual ~i_format_info(){}
    virtual bool is_encoded() const = 0;
    virtual bool is_convertable() const = 0;
    virtual bool is_planar() const = 0;
    virtual std::size_t frame_size() const = 0;
    virtual std::size_t planes() const = 0;
    virtual plane_sizes_t plane_sizes() const = 0;
    virtual std::string to_string() const = 0;
};

struct media_format_t : virtual public i_format_info
{
    const info_storage_ptr_t        info_storage;
    media_type_t                    media_type;
    extra_data_t                    extra_data;
    stream_id_t                     stream_id;
    std::string                     parameters;

    media_format_t(media_type_t media_type = media_type_t::data);

    media_format_t(const video::video_info_t& video_info
                   , stream_id_t stream_id = no_stream);

    media_format_t(const audio::audio_info_t& audio_info
                   , stream_id_t stream_id = no_stream);

    media_format_t(const data::data_info_t& data_info
                   , stream_id_t stream_id = no_stream);

    media_format_t(const media_format_t& media_format);
    media_format_t(media_format_t&& media_format) = default;

    media_format_t &operator=(const media_format_t& media_format);
    media_format_t &operator=(media_format_t&& media_format) = default;

    bool is_encoded() const override;
    bool is_convertable() const override;
    bool is_planar() const override;
    std::size_t frame_size() const override;
    std::size_t planes() const override;
    plane_sizes_t plane_sizes() const override;
    bool is_valid() const;

    virtual video::video_info_t& video_info();
    virtual audio::audio_info_t& audio_info();
    virtual data::data_info_t& data_info();

    virtual const video::video_info_t& video_info() const;
    virtual const audio::audio_info_t& audio_info() const;
    virtual const data::data_info_t& data_info() const;

    virtual bool operator ==(const media_format_t& media_format);
    virtual bool operator !=(const media_format_t& media_format);

    std::string to_string() const override;

};

struct external_media_info_t
{
    const void*     data;
    std::size_t     size;
    std::string     specific_options;
    external_media_info_t(void* data = nullptr
                          , std::size_t size = 0
                          , const std::string& specific_options = "");
};

}

}

#endif // MEDIA_INFO_H
