#ifndef AUDIO_CODEC_UTILS_H
#define AUDIO_CODEC_UTILS_H

#include "audio_codec_types.h"

#include <ostream>

std::ostream& operator<<(std::ostream& ostream, const largo::codec::audio::audio_codec_id_t& audio_codec_id);

namespace largo
{

namespace codec
{

namespace audio
{

	std::string get_codec_name_from_id(const largo::codec::audio::audio_codec_id_t& audio_codec_id);

} // audio

} // codec

} // largo

#endif // AUDIO_CODEC_UTILS_H
