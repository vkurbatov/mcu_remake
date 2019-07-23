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

class IBitstreamReader : virtual public IBitStream
{
public:
	virtual ~IBitstreamReader(){}
	virtual std::size_t Read(void* bit_data, std::size_t bit_count) = 0;
};

class IBitstreamWriter : virtual public IBitStream
{
public:
	virtual ~IBitstreamWriter(){}
	virtual std::size_t Write(const void* bit_data, std::size_t bit_count) = 0;
};

class IBitConverter
{
public:
	virtual ~IBitConverter(){}
	virtual void ReverseBits(std::int32_t bit_index, std::size_t bit_count) = 0;
	virtual void ConvertBle(std::int32_t byte_index, std::size_t byte_count) = 0;
};

class BitStreamReader : virtual public IBitstreamReader
{
	const void*			m_bit_stream;
	std::int32_t		m_bit_index;
	bool				m_big_endian_bits;


public:
	static std::size_t Read(const void* bit_stream, std::int32_t bit_index, void* bit_data, std::size_t bit_count, bool big_endian = false);

	template<typename T>
	static T ReadValue(void* bit_stream, std::int32_t bit_index, std::size_t bit_count = sizeof(T) * 8, bool big_endian = false);

public:
	BitStreamReader(const void* bit_stream, bool big_endian_bits = false);
	std::size_t Read(void* bit_data, std::size_t bit_count) override;

	template<typename T>
	T ReadValue(std::size_t bit_count = sizeof(T) * 8);

	std::int32_t GetBitIndex() const override;
	void Reset(std::int32_t bit_index = 0) override;

};

//------------------------------------------------------------------

class BitStreamWriter : virtual public IBitstreamWriter
{
	void*			m_bit_stream;
	std::int32_t	m_bit_index;
	bool			m_big_endian_bits;


public:
	static std::size_t Write(void* bit_stream, std::int32_t bit_index, const void* bit_data, std::size_t bit_count, bool big_endian = false);

	template<typename T>
	static void WriteValue(void* bit_stream, std::int32_t bit_index, const T& value, std::size_t bit_count = sizeof(T) * 8, bool big_endian = false);
public:
	BitStreamWriter(void* bit_stream, bool big_endian_bits = false);

	std::size_t Write(const void* bit_data, std::size_t bit_count) override;

	template<typename T>
	void WriteValue(const T& value, std::size_t bit_count = sizeof(T) * 8);

	std::int32_t GetBitIndex() const override;
	void Reset(std::int32_t bit_index = 0) override;
};

//------------------------------------------------------------------

class BitConverter : virtual public IBitConverter
{
	void*			m_bit_stream;
public:

	static void ReverseBits(void* bit_stream, int32_t bit_index, std::size_t bit_count);
	static void ConvertBle(void* bit_stream, int32_t byte_index, std::size_t byte_count);

	BitConverter(void* bit_stream);

	// IBitConverter interface
public:
	void ReverseBits(int32_t bit_index, std::size_t bit_count) override;
	void ConvertBle(int32_t byte_index, std::size_t byte_count) override;
};

} // largo

#endif // BIT_STREAM_H
