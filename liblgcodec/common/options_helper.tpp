#ifndef OPTIONS_HELPER_TPP
#define OPTIONS_HELPER_TPP

#include "options_helper.h"

namespace largo
{

// std::string template specialization

template<typename T>
T OptionsReader::GetOption(const IOptions& options, const option_key_t& key, const T& default_value)
{
	T result = default_value;

	options.GetOption(key, &result, sizeof(T));

	return result;
}

template<typename T>
T OptionsReader::GetOption(const option_key_t& key, const T& default_value) const
{
	return GetOption<T>(m_options, key, default_value);
}

//----------------------------------------------------------------------------------------------

template<typename T>
void OptionsWriter::SetOption(IOptions& options, const option_key_t& key, const T& value)
{
	options.SetOption(key, &value, sizeof(T));
}

template<typename T>
void OptionsWriter::SetOption(const option_key_t& key, const T& value)
{
	SetOption<T>(m_options, key, value);
}

} // helper

#endif
