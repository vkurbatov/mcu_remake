#ifndef MEDIA_CONTROL_PARAMETER_H
#define MEDIA_CONTROL_PARAMETER_H

#include "media/common/base/variant.h"
#include <map>
#include <functional>


namespace core
{

namespace media
{

using variant_type_t = base::variant_type_t;
using variant = base::variant;

typedef std::vector<variant> variant_list_t;

enum class control_type_t
{
    direct,
    list,
    check
};

class control_parameter;

typedef std::function<bool(variant& variant)> set_control_handler_t;
typedef std::function<bool(variant& variant)> get_control_handler_t;

typedef std::int32_t param_tag_t;
const param_tag_t custom_parameter = -1;


class control_parameter
{
    std::string                 m_name;
    variant_list_t              m_limits;
    mutable variant             m_value;
    control_type_t              m_control_type;
    param_tag_t                 m_tag;
    set_control_handler_t       m_set_handler;
    get_control_handler_t       m_get_handler;

public:
    control_parameter(const std::string& name = ""
                      , control_type_t control_type = control_type_t::direct
                      , const variant_list_t& limits = {}
                      , const variant& default_value = {}
                      , param_tag_t tag = custom_parameter
                      , set_control_handler_t set_handler = nullptr
                      , get_control_handler_t get_handler = nullptr);

    bool is_valid(const variant& value) const;

    bool set(const variant& value);
    void operator= (const variant& value);
    const variant& get() const;

    void bind_set_handler(set_control_handler_t set_handler);
    void bind_get_handler(get_control_handler_t get_handler);

    const std::string& name() const;
    const variant_list_t& limits() const;

    control_type_t type() const;

    const param_tag_t tag() const;

};

class control_parameter_list_t : public std::vector<control_parameter>
{
public:
    typename control_parameter_list_t::iterator find(const std::string& name);
    typename control_parameter_list_t::const_iterator find(const std::string& name) const;
    bool has_parameter(const std::string& name) const;
    bool set(const std::string& name, const variant& value);
    bool get(const std::string& name, variant& value) const;
};

//typedef std::vector<control_parameter> control_parameter_list_t;

void control_parameter_test();

}

}

#endif // MEDIA_CONTROL_PARAMETER_H
