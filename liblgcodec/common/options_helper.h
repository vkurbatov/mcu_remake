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
	static T GetOption(const IOptions& options, const option_key_t& key);

	template<typename T>
	static void SetOptions(IOptions& options, const option_key_t& key, T value);

public:
	OptionsHelper(IOptions& options) :
		m_options(options)
	{

	}

	template<typename T>
	T GetOption(const option_key_t& key);

	template<typename T>
	void SetOptions(const option_key_t& key, T value);

};

} // helper

#include "options_helper.tpp"

#endif // OPTIONS_HELPER_H
