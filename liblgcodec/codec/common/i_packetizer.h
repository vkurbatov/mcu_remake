#ifndef I_PACKETIZER_H
#define I_PACKETIZER_H

#include "i_codec_transcoder.h"

namespace largo
{

namespace codec
{

class IPacketizer : virtual public ICodecTranscoder
{
public:
	virtual ~IPacketizer() {}
	virtual std::size_t GetHeaderSize(std::size_t data_size = 0, const void* extra_data) const = 0;
};

} // codec

} //largo

#endif // I_PACKETIZER_H
