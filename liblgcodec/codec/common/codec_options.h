#ifndef CODEC_OPTIONS_H
#define CODEC_OPTIONS_H

#include "i_codec_options.h"
#include "common/options.h"

namespace largo
{

namespace codec
{

struct codec_options_t
{
	std::uint32_t sample_rate;
	std::uint32_t bit_rate;
};

class CodecOptions : virtual public Options
{
public:
	static const option_key_t codec_option_sample_rate;
	static const option_key_t codec_option_bit_rate;
public:
	CodecOptions(const codec_options_t& codec_options);
	virtual ~CodecOptions() override {}
};

} // codec

} // largo

#endif // CODEC_OPTIONS_H
