#ifndef OPTIONS_HELPER_TPP
#define OPTIONS_HELPER_TPP

#include "options_helper.h"
#include <cstring>

namespace largo
{

template<typename T>
T OptionsHelper::GetOption(const IOptions& options, const option_key_t& key, const T& default_value)
{
	T result = default_value;

	options.GetOption(key, &result, sizeof(T));

	return result;
}

template<typename T>
void OptionsHelper::SetOption(IOptions& options, const option_key_t& key, const T& value)
{
	options.SetOption(key, &value, sizeof(T));
}

// std::string template specialization

template<>
std::string OptionsHelper::GetOption(const IOptions& options, const option_key_t& key, const std::string& default_value)
{
	std::string result = default_value;

	auto meta = options[key];

	result = static_cast<const char*>(static_cast<const void*>(meta.meta_data.data()));

	return std::move(result);
}

template<>
void OptionsHelper::SetOption(IOptions& options, const option_key_t& key, const std::string& value)
{
	options.SetOption(key, value.data(), value.length() + 1);
}


//

template<typename T>
T OptionsHelper::GetOption(const option_key_t& key, const T& default_value) const
{
	return GetOption<T>(m_options, key, default_value);
}

template<typename T>
void OptionsHelper::SetOption(const option_key_t& key, const T& value)
{
	SetOption<T>(m_options, key, value);
}

} // helper

#endif
