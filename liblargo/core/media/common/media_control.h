#ifndef MEDIA_CONTROL_H
#define MEDIA_CONTROL_H

#include <string>
#include <vector>

namespace core
{

namespace media
{

enum class control_type_t
{
    range,
    list,
    flag
};

typedef std::vector<std::string> control_list_t;

typedef std::int32_t control_value_t;

const control_value_t default_min_value = 0;
const control_value_t default_max_value = 100;
const control_value_t default_value = default_min_value;

struct control_limits_t
{
    control_value_t min_value;
    control_value_t max_value;

    control_limits_t(control_value_t min_value = default_min_value
                     , control_value_t max_value = default_max_value);

    const control_value_t& operator()(control_value_t& value) const;
    control_value_t range() const;
    bool is_flag() const;
    bool is_in_range(const control_value_t& value) const;
};

struct media_control_t
{
    std::string         name;
    control_limits_t    limits;
    control_list_t      control_list;
    control_value_t           value;

    media_control_t(std::string name
                   , const control_limits_t& limits = control_limits_t()
                   , const control_list_t& control_list = control_list_t()
                   , control_value_t value = default_value);


    bool set_value(const std::string& new_value);
    bool set_value(control_value_t new_value);
    std::string to_string() const;

    control_type_t type() const;
    bool check_value(control_value_t checked_value) const;
    bool check_value(const std::string& new_value) const;
    bool is_valid() const;

};

typedef std::vector<media_control_t> media_control_list;

}

}

#endif // MEDIA_CONTROL_FORMAT_H
