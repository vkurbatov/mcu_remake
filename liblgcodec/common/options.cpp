#include "options.h"
#include <cstring>

namespace largo
{

Options::Options() :
	m_options_table()
{

}

const option_meta_info_t& Options::operator [](const option_key_t& key) const
{
	static option_meta_info_t empty_meta_data;

	const auto it = m_options_table.find(key);

	return it == m_options_table.end() ? empty_meta_data : it->second;
}

bool Options::GetOption(const option_key_t& key, void* option_data, std::size_t option_data_size, option_type_id_t type_id) const
{
	const auto it = m_options_table.find(key);

	bool result = it != m_options_table.end()
			&& (type_id == option_type_id_any || it->second.type_id == option_type_id_any || type_id == it->second.type_id)
			&& (option_data_size == 0 || option_data_size <= it->second.meta_data.size());

	if (result)
	{
		if (option_data != nullptr && option_data_size != 0)
		{
			std::memcpy(option_data, it->second.meta_data.data(), option_data_size);
		}
	}

	return result;

}

void Options::SetOption(const option_key_t& key, const void* option_data, std::size_t option_data_size, option_type_id_t type_id)
{

	option_meta_info_t& meta_info = m_options_table[key];

	meta_info.type_id = type_id;
	meta_info.meta_data.resize(option_data_size);
	std::memcpy(meta_info.meta_data.data(), option_data, option_data_size);

}

bool Options::RemoveOption(const option_key_t& key, option_type_id_t type_id)
{
	auto it = m_options_table.find(key);

	bool result = it != m_options_table.end() && (type_id == option_type_id_any || it->second.type_id == option_type_id_any || type_id == it->second.type_id);

	if (result)
	{
		m_options_table.erase(it);
	}

	return result;
}

bool Options::HasOption(const option_key_t& key, option_type_id_t type_id) const
{
	const auto& it = m_options_table.find(key);

	return it != m_options_table.end() && (type_id == option_type_id_any || it->second.type_id == option_type_id_any || type_id == it->second.type_id);
}

void Options::Clear()
{
	m_options_table.clear();
}

} // largo
