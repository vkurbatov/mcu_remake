#ifndef BIT_STREAM_TPP
#define BIT_STREAM_TPP

#include "bit_stream.h"

namespace largo
{

template<typename T>
T BitStreamReader::ReadValue(void* bit_stream, std::int32_t bit_index, std::size_t bit_count)
{
	T result = {};

	Read(bit_stream, bit_index, static_cast<void*>(&result), bit_count);

	return result;
}

template<typename T>
T BitStreamReader::ReadValue(std::size_t bit_count)
{
	T result = {};

	BitStreamReader::Read(&result, bit_count);

	return result;
}

//----------------------------------------------------------------
template<typename T>
void BitStreamWriter::WriteValue(void* bit_stream, std::int32_t bit_index, const T& value, std::size_t bit_count)
{
	 Write(bit_stream, bit_index, static_cast<const void*>(&value), bit_count);
}

template<typename T>
void BitStreamWriter::WriteValue(const T& value, std::size_t bit_count)
{
	BitStreamWriter::Write(static_cast<const void*>(&value), bit_count);
}

} // helper

#endif
