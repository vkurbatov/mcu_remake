#ifndef I_MEDIA_STREAM_H
#define I_MEDIA_STREAM_H

#include "core/media/common/i_media_point.h"
//#include <cstdint>

namespace core
{

namespace media
{

using media_stream_id_t = std::uint32_t;

const media_stream_id_t media_stream_id_none = 0;
const media_stream_id_t media_stream_id_min = 1;

class IMediaStream : virtual public IMediaPoint
{
public:
	virtual ~IMediaStream() {}

	virtual media_stream_id_t GetStreamId() const = 0;

};

} // media

} // core

#endif // I_MEDIA_STREAM_H
