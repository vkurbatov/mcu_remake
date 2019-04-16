#include "codec_options.h"

namespace largo
{

namespace codec
{

const option_key_t CodecOptions::codec_option_sample_rate = "codec.sample_rate";
const option_key_t CodecOptions::codec_option_bit_rate = "codec.bit_rate";


CodecOptions::CodecOptions(const codec_options_t& codec_options)
{
	SetOption(codec_option_sample_rate, &codec_options.sample_rate, sizeof(codec_options.sample_rate));
	SetOption(codec_option_bit_rate, &codec_options.bit_rate, sizeof(codec_options.bit_rate));
}

} // codec

} // largo
