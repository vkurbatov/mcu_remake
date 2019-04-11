#ifndef I_OPTIONS_H
#define I_OPTIONS_H

#include <cstdint>
#include <string>

namespace largo
{

using option_key_t = std::string;
using option_type_id_t = std::size_t;
const option_type_id_t any_option_type = 0;

struct option_meta_data_t
{
	option_type_id_t	type_id;
	std::size_t			size;
};

class IOptions
{
public:
	virtual ~IOptions() {}
	virtual const option_type_id_t& GetOptionMetaData() const = 0;
	virtual bool GetOption(const option_key_t& key, void* data, std::size_t size, option_type_id_t type = any_option_type) = 0;
	virtual void SetOption(const option_key_t& key, const void* data, std::size_t size, option_type_id_t type = any_option_type) = 0;
	//virtual bool GetOption(const option_key_t& key, void* data, std::size_t size);

};

}

#endif
