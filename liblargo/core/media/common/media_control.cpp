#include "media_control.h"
#include <cstring>

namespace core
{

namespace media
{

template<typename Tin, typename Tout>
Tout convert(const Tin& value)
{
    return static_cast<Tout>(value);
}

template<typename Tin>
std::string convert(const Tin& value)
{
    return std::to_string(value);
}

template<>
string_value_t convert(const string_value_t& value)
{
    return value;
}

template<typename T>
value_data_t::value_data_t(const T &value)
{
    operator =(value);
}

template<typename T>
value_data_t &value_data_t::operator=(const T &value)
{
    data.resize(sizeof(T));
    reinterpret_cast<T&>(*data.data()) = value;
    return *this;
}

template<>
value_data_t &value_data_t::operator=(const std::string &value)
{
    data.resize(value.size());
    std::memcpy(data.data()
                , value.data()
                , value.size());

    return *this;
}

template<typename T>
T value_data_t::get(const T &default_value) const
{
    if (data.size() != 0)
    {
        return reinterpret_cast<const T&>(*data.data());
    }

    return default_value;
}

template<>
std::string value_data_t::get(const std::string &/*default_value*/) const
{
    std::string result;

    result.resize(data.size());

    std::memcpy(&result[0]
                , data.data()
                , data.size());

    return result;
}

//----------------------------------------------------------------------------
template<>
control_value_t::control_value_t(const numeric_value_t &value)
    : value_data(value)
{

}

template<>
control_value_t::control_value_t(const real_value_t &value)
    : value_data(value)
{

}
template<>
control_value_t::control_value_t(const string_value_t &value)
    : value_data(value)
{

}
template<>
control_value_t::control_value_t(const flag_value_t &value)
    : value_data(value)
{

}

template<>
control_value_t &control_value_t::operator=(const numeric_value_t &value)
{

}

}

}
