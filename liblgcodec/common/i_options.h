#ifndef I_OPTIONS_H
#define I_OPTIONS_H

#include <cstdint>
#include <string>
#include <vector>

namespace largo
{

using option_key_t = std::string;
using option_type_id_t = std::size_t;
const option_type_id_t option_type_id_any = 0;

struct option_meta_info_t
{
	option_type_id_t			type_id;
	std::vector<std::uint8_t>	meta_data;

	option_meta_info_t(option_type_id_t t_id = option_type_id_any) : type_id(t_id)
		, meta_data()
	{

	}

	option_meta_info_t(const void* dt, std::size_t sz, option_type_id_t t_id = option_type_id_any) : type_id(t_id)
		, meta_data(static_cast<const std::uint8_t*>(dt), static_cast<const std::uint8_t*>(dt) + sz)
	{

	}

	bool is_empty() const
	{
		return type_id == option_type_id_any && meta_data.empty();
	}
};

class IOptions
{
public:
	virtual ~IOptions() {}
	virtual const option_meta_info_t& operator [] (const option_key_t& key) const = 0;
	virtual bool GetOption(const option_key_t& key, void* option_data, std::size_t option_data_size = 0, option_type_id_t type_id = option_type_id_any) const = 0;
	virtual void SetOption(const option_key_t& key, const void* option_data, std::size_t option_data_size, option_type_id_t type_id = option_type_id_any) = 0;
	virtual bool RemoveOption(const option_key_t& key, option_type_id_t type_id = option_type_id_any) = 0;
	virtual bool HasOption(const option_key_t& key, option_type_id_t type_id = option_type_id_any) const = 0;
	virtual std::size_t MergeFrom(const IOptions& options) = 0;
	virtual std::size_t MergeTo(IOptions& options) const = 0;
	virtual std::size_t Size() const = 0;
	virtual void Clear() = 0;
	//virtual bool GetOption(const option_key_t& key, void* data, std::size_t size);

};

}

#endif
