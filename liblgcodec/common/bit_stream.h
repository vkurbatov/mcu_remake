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

	template<typename T>
	static T Read(void* bit_stream, std::int32_t bit_index, std::size_t bit_count = sizeof(T) * 8);

public:
	BitStreamReader(const void* bit_stream);
	std::size_t Read(void* bit_data, std::size_t bit_count);

	template<typename T>
	T Read(std::size_t bit_count = sizeof(T) * 8);

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

	template<typename T>
	static void Write(void* bit_stream, std::int32_t bit_index, const T& value, std::size_t bit_count = sizeof(T) * 8);
public:
	BitStreamWriter(void* bit_stream);

	std::size_t Write(const void* bit_data, std::size_t bit_count);

	template<typename T>
	void Write(const T& value, std::size_t bit_count = sizeof(T) * 8);

	std::int32_t GetBitIndex() const;
	void Reset(std::int32_t bit_index = 0);
};

} // largo

#endif // BIT_STREAM_H