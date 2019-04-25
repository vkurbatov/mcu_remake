#include "audio_codec_utils.h"

std::ostream& operator<<(std::ostream& ostream, const largo::codec::audio::audio_codec_id_t& audio_codec_id)
{
	return ostream << largo::codec::audio::get_codec_name_from_id(audio_codec_id);
}

namespace largo
{

namespace codec
{

namespace audio
{

std::string get_codec_name_from_id(const audio_codec_id_t& audio_codec_id)
{
	static const char* codec_string[] = {
			"Unknown",
			"G723",
			"AAC LD"};

	return codec_string[static_cast<std::int32_t>(audio_codec_id)];
}

} // audio

} // codec

} // largo

