#ifndef OPTIONS_H
#define OPTIONS_H

#include "i_options.h"
#include <unordered_map>

#define GET_OPTION(prefix_key, options, param_base, param) options.GetOption(prefix_key##param, &param_base.param, sizeof(param_base.param))
#define SET_OPTION(prefix_key, options, param_base, param) options.SetOption(prefix_key##param, &param_base.param, sizeof(param_base.param));

namespace largo
{

class Options : virtual public IOptions
{
	using options_table_t = std::unordered_map<option_key_t, option_meta_info_t>;
	options_table_t	m_options_table;

public:
	Options();
	virtual ~Options() {}
	// IOptions interface
public:
	const option_meta_info_t& operator [] (const option_key_t& key) const override;
	bool GetOption(const option_key_t& key, void* option_data, std::size_t option_data_size = 0, option_type_id_t type_id = option_type_id_any) const override;
	bool GetOption(const option_key_t& key, std::vector<std::uint8_t>& option_data, option_type_id_t type_id = option_type_id_any) const override;
	void SetOption(const option_key_t& key, const void* option_data, std::size_t option_data_size, option_type_id_t type_id = option_type_id_any) override;
	void SetOption(const option_key_t& key, const std::vector<std::uint8_t>& option_data, option_type_id_t type_id = option_type_id_any) override;
	bool RemoveOption(const option_key_t& key, option_type_id_t type_id = option_type_id_any) override;
	bool HasOption(const option_key_t& key, option_type_id_t type_id = option_type_id_any) const override;
	IOptions& operator << (const IOptions& options) override;
	IOptions& operator >> (IOptions& options) const override;
	std::size_t Size() const override;
	void Clear() override;
};

} // largo

#endif // OPTIONS_H
