#ifndef MEDIA_CONTROL_H
#define MEDIA_CONTROL_H

#include <string>
#include <vector>

namespace core
{

namespace media
{

enum class control_value_type_t
{
    number,
    real,
    string,
    flag
};

typedef std::int64_t numeric_value_t;
typedef double real_value_t;
typedef std::string string_value_t;
typedef bool flag_value_t;

struct value_data_t
{
    std::vector<std::uint8_t> data;

    template<typename T>
    value_data_t(const T& value);

    template<typename T>
    value_data_t& operator=(const T& value);

    template<typename T>
    T get(const T& default_value = {}) const;

};

struct control_value_t
{
    control_value_type_t type;

    value_data_t value_data;
    template<typename T>
    control_value_t(const T& value);

    template<typename T>
    control_value_t& operator=(const T& value);

    template<typename T>
    T get(const T& default_value = {}) const;

};

}

}

#endif // MEDIA_CONTROL_FORMAT_H
