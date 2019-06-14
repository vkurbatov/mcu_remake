#ifndef BIT_STREAM_H
#define BIT_STREAM_H

#include <cstdint>

namespace largo
{

class IBitStream
{
public:
	virtual ~IBitStream(){}
	virtual std::int32_t GetBitIndex() const = 0;
	virtual void Reset(std::int32_t bit_index = 0) = 0;
};

class BitStreamReader : virtual public IBitStream
{
	const void*			m_bit_stream;
	std::int32_t		m_bit_index;

public:
	static std::size_t Read(const void* bit_stream, std::int32_t bit_index, void* bit_data, std::size_t bit_count);

	template<typename T>
	static T ReadValue(void* bit_stream, std::int32_t bit_index, std::size_t bit_count = sizeof(T) * 8);

public:
	BitStreamReader(const void* bit_stream);
	std::size_t Read(void* bit_data, std::size_t bit_count);

	template<typename T>
	T ReadValue(std::size_t bit_count = sizeof(T) * 8);

	std::int32_t GetBitIndex() const override;
	void Reset(std::int32_t bit_index = 0) override;

};

//------------------------------------------------------------------

class BitStreamWriter : virtual public IBitStream
{
	void*			m_bit_stream;
	std::int32_t	m_bit_index;

public:
	static std::size_t Write(void* bit_stream, std::int32_t bit_index, const void* bit_data, std::size_t bit_count);

	template<typename T>
	static void WriteValue(void* bit_stream, std::int32_t bit_index, const T& value, std::size_t bit_count = sizeof(T) * 8);
public:
	BitStreamWriter(void* bit_stream);

	std::size_t Write(const void* bit_data, std::size_t bit_count);

	template<typename T>
	void WriteValue(const T& value, std::size_t bit_count = sizeof(T) * 8);

	std::int32_t GetBitIndex() const override;
	void Reset(std::int32_t bit_index = 0) override;
};

} // largo

#endif // BIT_STREAM_H
