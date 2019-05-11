#include "options_helper.h"
#include <cstring>

namespace largo
{

// std::string template specialization

template<>
std::string OptionsReader::GetOption(const IOptions& options, const option_key_t& key, const std::string& default_value)
{
	std::string result = default_value;

	auto meta = options[key];

	result = static_cast<const char*>(static_cast<const void*>(meta.meta_data.data()));

	return std::move(result);
}

OptionsReader::OptionsReader(const IOptions &options)
	: m_options(options)
{

}

//-------------------------------------------------------------------------------------------

template<>
void OptionsWriter::SetOption(IOptions& options, const option_key_t& key, const std::string& value)
{
	options.SetOption(key, value.data(), value.length() + 1);
}

OptionsWriter::OptionsWriter(IOptions &options)
	: m_options(options)
{

}

} // largo
