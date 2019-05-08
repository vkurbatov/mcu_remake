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

inline void copy_bit(const std::uint8_t* bit_src_data, uint32_t bit_src_idx, std::uint8_t* bit_dst_data, uint32_t bit_dst_idx = 0)
{

	if ((*bit_src_data & (1 << bit_src_idx)) == 0)
	{
		*bit_dst_data &= ~(1 << bit_dst_idx);
	}
	else
	{
		*bit_dst_data |= (1 << bit_dst_idx);
	}
}

void copy_bits(const void* bit_src_data, int32_t bit_src_idx, void* bit_dst_data, int32_t bit_dst_idx = 0, std::size_t count = 1)
{
	auto src_data = static_cast<const std::uint8_t*>(bit_src_data);
	auto dst_data = static_cast<std::uint8_t*>(bit_dst_data);

	while (count > 0)
	{
		src_data += normalize_index(bit_src_idx);
		dst_data += normalize_index(bit_dst_idx);

		copy_bit(src_data, bit_src_idx, dst_data, bit_dst_idx);

		bit_src_idx++;
		bit_dst_idx++;

		count--;
	}
}

} // bit_stream_utils

std::size_t BitStreamReader::Read(const void* bit_stream, int32_t bit_index, void* bit_data, std::size_t bit_count)
{
	bit_stream_utils::copy_bits(bit_stream, bit_index, bit_data, 0, bit_count);

	return bit_count;
}

BitStreamReader::BitStreamReader(const void* bit_stream)
	: m_bit_stream(bit_stream)
	, m_bit_index(0)
{

}

std::size_t BitStreamReader::Read(void* bit_data, std::size_t bit_count)
{
	auto result = bit_data != nullptr ? Read(m_bit_stream, m_bit_index, bit_data, bit_count) : bit_count;

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

std::size_t BitStreamWriter::Write(void* bit_stream, int32_t bit_index, const void* bit_data, std::size_t bit_count)
{
	bit_stream_utils::copy_bits(bit_data, 0, bit_stream, bit_index, bit_count);

	return bit_count;
}

BitStreamWriter::BitStreamWriter(void* bit_stream)
	: m_bit_stream(bit_stream)
	, m_bit_index(0)
{

}

std::size_t BitStreamWriter::Write(const void* bit_data, std::size_t bit_count)
{
	auto result = Write(m_bit_stream, m_bit_index, bit_data, bit_count);

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

} //largo
