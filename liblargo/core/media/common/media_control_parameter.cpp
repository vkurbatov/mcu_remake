#include "media_control_parameter.h"
#include <cstring>
#include <algorithm>

namespace core
{

namespace media
{

static bool is_in_range(const variant& value
                        , const variant_list_t& limits)
{
    return limits.size() != 2
            || (limits[0] >= value
                && limits[1] <= value);
}

static bool is_in_list(const variant& value
                       , const variant_list_t& limits)
{
    for (const auto& v : limits)
    {
        if (v == value)
        {
            return true;
        }
    }
    return false;
}

static const variant& clamp_direct_value(const variant& value
                                         , const variant_list_t& limits)
{
    if (limits.size() == 2)
    {
        if (value.type() == variant_type_t::vt_unknown)
        {
            return limits[0];
        }
        return std::min(std::max(value, limits[0]), limits[1]);
    }

    return value;
}

static const variant& init_value(const variant& default_value
                                 , control_type_t control_type
                                 , const variant_list_t& limits)
{
    switch(control_type)
    {
        case control_type_t::direct:
            return clamp_direct_value(default_value
                                      , limits);
        break;
        case control_type_t::list:
            return limits.empty()
                    || is_in_list(default_value, limits)
                    ? default_value
                    : limits[0];
        break;
        case control_type_t::check:
            return default_value;
        break;
    }

    return default_value;
}

control_parameter::control_parameter(const std::string& name
                                     , control_type_t control_type
                                     , const variant_list_t &limits
                                     , const variant &default_value
                                     , param_tag_t tag
                                     , set_control_handler_t set_handler
                                     , get_control_handler_t get_handler)
    : m_name(name)
    , m_control_type(control_type)
    , m_limits(limits)
    , m_value(init_value(default_value
                         , control_type
                         , limits))
    , m_tag(tag)
    , m_set_handler(set_handler)
    , m_get_handler(get_handler)
{

}

bool control_parameter::is_valid(const variant &value) const
{
    switch(m_control_type)
    {
        case control_type_t::direct:
            return is_in_range(value
                               , m_limits);
        break;
        case control_type_t::list:
            return is_in_list(value
                              , m_limits);
        break;
        case control_type_t::check:
            return true;
        break;
    }

    return false;
}

bool control_parameter::set(const variant &value)
{
    switch(m_control_type)
    {
        case control_type_t::direct:
            m_value = clamp_direct_value(value, m_limits);
        break;
        case control_type_t::list:
            if (!is_valid(value))
            {
                return false;
            }
            m_value = value;
        break;
        case control_type_t::check:
            m_value = value;
        break;
    }        

    return m_set_handler == nullptr
            || m_set_handler(m_value);
}

void control_parameter::operator=(const variant &value)
{
    set(value);
}

const variant& control_parameter::get() const
{
    if (m_get_handler != nullptr)
    {
        m_get_handler(m_value);
    }
    return m_value;
}

void control_parameter::bind_get_handler(get_control_handler_t get_handler)
{
    m_get_handler = get_handler;
}

void control_parameter::bind_set_handler(set_control_handler_t set_handler)
{
    m_set_handler = set_handler;
}

const std::string &control_parameter::name() const
{
    return m_name;
}

const variant_list_t &control_parameter::limits() const
{
    return m_limits;
}

control_type_t control_parameter::type() const
{
    return m_control_type;
}

const param_tag_t control_parameter::tag() const
{
    return m_tag;
}

void control_parameter_test()
{
    control_parameter parameter_list("list_param"
                                     , control_type_t::list
                                     , { "vaslily" , "kurbatov" });

    auto cur = parameter_list.get();

    parameter_list = "kurbatov";

    cur = parameter_list.get();

    parameter_list = "жопа";

    cur = parameter_list.get();

    control_parameter parameter_direct("string_param"
                                        , control_type_t::direct
                                        , { "45" , "98" });

    cur = parameter_direct.get().get<std::int32_t>();

    parameter_direct.set(47);

    cur = parameter_direct.get();

    parameter_direct = 2;

    cur = parameter_direct.get();

    return;
}

control_parameter_list_t::iterator control_parameter_list_t::find(const std::string &name)
{
    return std::find_if(begin(), end(), [&name](const control_parameter& parameter) { return parameter.name() == name; });
}

control_parameter_list_t::const_iterator control_parameter_list_t::find(const std::string &name) const
{
    return std::find_if(begin(), end(), [&name](const control_parameter& parameter) { return parameter.name() == name; });
}

bool control_parameter_list_t::has_parameter(const std::string &name) const
{
    return find(name) != end();
}

bool control_parameter_list_t::set(const std::string &name, const variant &value)
{
    auto it = find(name);

    if (it != end())
    {
        return (*it).set(value);
    }

    return false;
}

bool control_parameter_list_t::get(const std::string &name, variant &value) const
{
    auto it = find(name);

    if (it != end())
    {
        value = (*it).get();
        return true;
    }

    return false;
}

}

}
