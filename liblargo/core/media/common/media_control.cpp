#include "media_control.h"

namespace core
{

namespace media
{


control_limits_t::control_limits_t(control_value_t min_value
                                   , control_value_t max_value)
    : min_value(min_value)
    , max_value(max_value)
{

}

const control_value_t& control_limits_t::operator()(control_value_t &value) const
{
    return std::min(max_value, std::max(min_value, value));
}

control_value_t control_limits_t::range() const
{
    return max_value - min_value;
}

bool control_limits_t::is_flag() const
{
    return min_value == 0
            && (max_value == 0
                || max_value == 1);
}

bool control_limits_t::is_in_range(const control_value_t &value) const
{
    return value >= min_value
            && value <= max_value;
}

media_control_t::media_control_t(std::string name
                                 , const control_limits_t &limits
                                 , const control_list_t &control_list
                                 , control_value_t value)
    : name(name)
    , limits(limits)
    , control_list(control_list)
    , value(this->limits(value))
{

}

bool media_control_t::set_value(const std::string &new_value)
{
    if (type() == control_type_t::list)
    {
        auto i = 0;
        for (const auto& item : control_list)
        {
            if (item == new_value)
            {
                value = i;
                return true;
            }
        }
    }
    else
    {
        try
        {
            auto v = std::stoi(new_value);
            if (limits.is_in_range(v))
            {
                value = v;
                return true;
            }
        }
        catch(...)
        {
            //
        }
    }

    return false;
}

bool media_control_t::set_value(control_value_t new_value)
{
    if (check_value(new_value))
    {
        value = new_value;
        return true;
    }

    return false;
}

std::string media_control_t::to_string() const
{
    if (is_valid()
            && type() == control_type_t::list)
    {
        return control_list[value];
    }

    return std::to_string(value);
}

control_type_t media_control_t::type() const
{
    control_type_t control_type = limits.is_flag()
            ? control_type_t::flag
            : control_type_t::range;

    if (control_list.size() > 1)
    {
        control_type = control_type_t::list;
    }

    return control_type;
}

bool media_control_t::check_value(control_value_t checked_value) const
{
    return (type() == control_type_t::list
            && checked_value >= 0
            && checked_value < control_list.size())
            || limits.is_in_range(checked_value);
}

bool media_control_t::check_value(const std::string &new_value) const
{
    if (type() == control_type_t::list)
    {
        for (const auto& item : control_list)
        {
            if (item == new_value)
            {
                return true;
            }
        }
    }
    else
    {
        try
        {
            return limits.is_in_range(std::stoi(new_value));
        }
        catch(...)
        {
            //
        }
    }

    return false;
}

bool media_control_t::is_valid() const
{
    return check_value(value);
}

}

}
