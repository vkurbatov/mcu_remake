#ifndef BIT_STREAM_H
#define BIT_STREAM_H

#include <cstdint>

namespace largo
{

class BitStreamReader
{
	const void*			m_bit_stream;
	std::int32_t		m_bit_index;

public:
	static std::size_t Read(const void* bit_stream, std::int32_t bit_index, void* bit_data, std::size_t bit_count);

public:
	BitStreamReader(const void* bit_stream);
	std::size_t Read(void* bit_data, std::size_t bit_count);

	template<typename T>
	T Read(std::size_t bit_count = sizeof(T))
	{
		T result = {};

		BitStreamReader::Read(&result, bit_count);

		return result;
	}

	std::int32_t GetBitIndex() const;
	void Reset(std::int32_t bit_index = 0);

};

//------------------------------------------------------------------

class BitStreamWriter
{
	void*			m_bit_stream;
	std::int32_t	m_bit_index;

public:
	static std::size_t Write(void* bit_stream, std::int32_t bit_index, const void* bit_data, std::size_t bit_count);

	BitStreamWriter(void* bit_stream);

	std::size_t Write(const void* bit_data, std::size_t bit_count);

	template<typename T>
	void Write(const T& value, std::size_t bit_count = sizeof(T))
	{
		BitStreamWriter::Write(static_cast<const void*>(&value), bit_count);
	}

	std::int32_t GetBitIndex() const;
	void Reset(std::int32_t bit_index = 0);
};

} // largo

#endif // BIT_STREAM_H
