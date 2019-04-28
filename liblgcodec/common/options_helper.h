#ifndef OPTIONS_HELPER_H
#define OPTIONS_HELPER_H

#include "i_options.h"

namespace largo
{

class OptionsHelper
{

	IOptions&	m_options;

public:
	template<typename T>
	static T GetOption(const IOptions& options, const option_key_t& key, const T& default_value = {});


	template<typename T>
	static void SetOption(IOptions& options, const option_key_t& key, const T& value);

public:
	OptionsHelper(IOptions& options);

	template<typename T>
	T GetOption(const option_key_t& key, const T& default_value = {}) const;

	template<typename T>
	void SetOption(const option_key_t& key, const T& value);

};

} // helper

#include "options_helper.tpp"

#endif // OPTIONS_HELPER_H
