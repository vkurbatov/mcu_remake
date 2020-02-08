#ifndef MEDIA_CONTROL_PARAMETER_H
#define MEDIA_CONTROL_PARAMETER_H

#include "media/common/base/variant.h"
#include <map>


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


class control_parameter
{
    std::string     m_name;
    variant_list_t  m_limits;
    variant         m_value;
    control_type_t  m_control_type;

public:
    control_parameter(const std::string& name = ""
                      , control_type_t control_type = control_type_t::direct
                      , const variant_list_t& limits = {}
                      , const variant& default_value = {});

    bool is_valid(const variant& value) const;

    bool set(const variant& value);
    void operator= (const variant& value);

    const variant& get() const;


    const std::string& name() const;
    const variant_list_t& limits() const;

    control_type_t type() const;

};


typedef std::vector<control_parameter> control_parameter_list_t;

void control_parameter_test();

}

}

#endif // MEDIA_CONTROL_PARAMETER_H
