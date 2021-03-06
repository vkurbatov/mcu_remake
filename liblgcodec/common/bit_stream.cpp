#include "bit_stream.h"

namespace largo
{

const std::int32_t bit_per_byte = 8;

namespace bit_stream_utils
{

inline std::int32_t normalize_index(int32_t& bit_idx)
{
	std::int32_t bit_idx_offset = bit_idx < 0 ? (bit_per_byte - 1) : 0;


	std::int32_t result = (bit_idx - bit_idx_offset) / bit_per_byte;

	bit_idx -= result * bit_per_byte;

	return result;
}

inline bool get_bit(const std::uint8_t* bit_src_data, uint32_t bit_src_idx, bool be = false)
{
	auto byte_idx = bit_src_idx / bit_per_byte;
	auto bit_idx = be ? (7 - bit_src_idx % bit_per_byte) : bit_src_idx % bit_per_byte;

	return (bit_src_data[byte_idx] & (1 << bit_idx)) != 0;
}

inline void set_bit(std::uint8_t* bit_dst_data, uint32_t bit_dst_idx, bool value, bool be = false)
{
	auto byte_idx = bit_dst_idx / bit_per_byte;
	auto bit_idx = be ? (7 - bit_dst_idx % bit_per_byte) : bit_dst_idx % bit_per_byte;

	if (value == false)
	{
		bit_dst_data[byte_idx] &= ~(1 << (bit_idx));
	}
	else
	{
		bit_dst_data[byte_idx] |= (1 << (bit_idx));
	}
}

inline void copy_bit(const std::uint8_t* bit_src_data, uint32_t bit_src_idx, std::uint8_t* bit_dst_data, uint32_t bit_dst_idx = 0, bool src_be = false, bool dst_be = false)
{
	set_bit(bit_dst_data, bit_dst_idx, get_bit(bit_src_data, bit_src_idx, src_be), dst_be);
}

void copy_bits(const void* bit_src_data, int32_t bit_src_idx, void* bit_dst_data, int32_t bit_dst_idx = 0, std::size_t count = 1, bool src_be = false, bool dst_be = false)
{
	auto src_data = static_cast<const std::uint8_t*>(bit_src_data);
	auto dst_data = static_cast<std::uint8_t*>(bit_dst_data);

	if (dst_be)
	{
		bit_src_idx = count - 1;
	}
	if (src_be)
	{
		bit_dst_idx = count - 1;
	}

	while (count > 0)
	{
		src_data += normalize_index(bit_src_idx);
		dst_data += normalize_index(bit_dst_idx);

		copy_bit(src_data, bit_src_idx, dst_data, bit_dst_idx, src_be, dst_be);

		dst_be ? bit_src_idx-- : bit_src_idx++;
		src_be ? bit_dst_idx-- : bit_dst_idx++;

		count--;
	}
}

} // bit_stream_utils

// ---------------------------------------------------------------------------------------

std::size_t BitStreamReader::Read(const void* bit_stream, int32_t bit_index, void* bit_data, std::size_t bit_count, bool big_endian)
{

	bit_stream_utils::copy_bits(bit_stream, bit_index, bit_data, 0, bit_count, big_endian);

	return bit_count;
}

BitStreamReader::BitStreamReader(const void* bit_stream, bool big_endian_bits)
	: m_bit_stream(bit_stream)
	, m_bit_index(0)
	, m_big_endian_bits(big_endian_bits)
{

}

std::size_t BitStreamReader::Read(void* bit_data, std::size_t bit_count)
{
	auto result = bit_data != nullptr ? Read(m_bit_stream, m_bit_index, bit_data, bit_count, m_big_endian_bits) : bit_count;

	m_bit_index += static_cast<std::int32_t>(result);

	return result;
}

int32_t BitStreamReader::GetBitIndex() const
{
	return m_bit_index;
}

void BitStreamReader::Reset(int32_t bit_index)
{
	m_bit_index = bit_index;
}

// ---------------------------------------------------------------------------------------

std::size_t BitStreamWriter::Write(void* bit_stream, int32_t bit_index, const void* bit_data, std::size_t bit_count, bool big_endian)
{

	bit_stream_utils::copy_bits(bit_data, bit_index, bit_stream, bit_index, bit_count, false, big_endian);

	return bit_count;
}

BitStreamWriter::BitStreamWriter(void* bit_stream, bool big_endian_bits)
	: m_bit_stream(bit_stream)
	, m_bit_index(0)
	, m_big_endian_bits(big_endian_bits)
{

}

std::size_t BitStreamWriter::Write(const void* bit_data, std::size_t bit_count)
{

	auto result = Write(m_bit_stream, m_bit_index, bit_data, bit_count, m_big_endian_bits);

	m_bit_index += static_cast<std::int32_t>(result);

	return result;
}

int32_t BitStreamWriter::GetBitIndex() const
{
	return m_bit_index;
}

void BitStreamWriter::Reset(int32_t bit_index)
{
	m_bit_index = bit_index;
}

void BitConverter::ReverseBits(void* bit_stream, std::int32_t bit_index, std::size_t bit_count)
{
	auto bit_data = static_cast<std::uint8_t*>(bit_stream);
	auto end_index = bit_index + bit_count - 1;

	while (bit_index < end_index)
	{
		auto tmp_bit = bit_stream_utils::get_bit(bit_data, bit_index);

		bit_stream_utils::set_bit(bit_data, bit_index, bit_stream_utils::get_bit(bit_data, end_index));
		bit_stream_utils::set_bit(bit_data, end_index, tmp_bit);

		bit_index++;
		end_index--;
	}
}

void BitConverter::ConvertBle(void* bit_stream, std::int32_t byte_index, std::size_t byte_count)
{
	auto bit_data = static_cast<std::uint8_t*>(bit_stream);

	while(byte_count-- > 0)
	{
		ReverseBits(bit_data, byte_index * bit_per_byte, bit_per_byte);
		byte_index++;
	}
}

BitConverter::BitConverter(void* bit_stream)
	: m_bit_stream(bit_stream)
{

}

void BitConverter::ReverseBits(int32_t bit_index, std::size_t bit_count)
{
	ReverseBits(m_bit_stream, bit_index, bit_count);
}

void BitConverter::ConvertBle(int32_t byte_index, std::size_t byte_count)
{
	ConvertBle(m_bit_stream, byte_index, byte_count);
}

} //largo
