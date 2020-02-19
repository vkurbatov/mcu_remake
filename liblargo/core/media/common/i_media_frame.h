#ifndef I_MEDIA_FRAME_H
#define I_MEDIA_FRAME_H

#include "media_format.h"
#include "i_media_plane.h"
#include "i_media_buffer.h"

#include <memory>
#include <vector>

namespace core
{

namespace media
{

typedef std::unique_ptr<i_media_plane> media_plane_ptr_t;
typedef std::vector<media_plane_ptr_t> media_plane_list_t;

typedef std::uint32_t frame_id_t;
typedef std::uint64_t timestamp_t;

class i_media_frame;

typedef std::shared_ptr<i_media_frame> media_frame_ptr_t;

class i_media_frame
{  
public:
    virtual ~i_media_frame() {}
    virtual const media_format_t& media_format() const = 0;
    virtual media_plane_list_t planes() const = 0;
    virtual std::size_t size() const = 0;
    virtual bool is_valid() const = 0;    
    virtual frame_id_t frame_id() const = 0;
    virtual timestamp_t timestamp() const = 0;
    virtual media_frame_ptr_t clone() const = 0;

    virtual void clear() = 0;
    virtual void swap(media_buffer_ptr_t&& media_buffer) = 0;
    virtual media_buffer_ptr_t release() = 0;
};


}

}

#endif // I_MEDIA_FRAME_H
