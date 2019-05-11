#ifndef OPTIONS_HELPER_H
#define OPTIONS_HELPER_H

#include "i_options.h"

namespace largo
{

class OptionsReader
{
	const IOptions&	m_options;
public:
	template<typename T>
	static T GetOption(const IOptions& options, const option_key_t& key, const T& default_value = {});
public:
	OptionsReader(const IOptions& options);

	template<typename T>
	T GetOption(const option_key_t& key, const T& default_value = {}) const;
};

class OptionsWriter
{
	 IOptions&	m_options;
 public:
	 template<typename T>
	 static void SetOption(IOptions& options, const option_key_t& key, const T& value);
 public:
	 OptionsWriter(IOptions& options);

	 template<typename T>
	 void SetOption(const option_key_t& key, const T& value);
};

} // largo

#include "options_helper.tpp"

#endif // OPTIONS_HELPER_H
