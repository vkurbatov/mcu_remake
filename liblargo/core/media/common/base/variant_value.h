#ifndef VARIANT_VALUE_H
#define VARIANT_VALUE_H

#include <vector>
#include <string>

namespace base
{

typedef std::int64_t numeric_type_t;
typedef double real_type_t;
typedef std::string string_type_t;
typedef bool boolean_type_t;

typedef std::vector<std::uint8_t> storage_value_t;

enum class variant_type_t
{
    numeric,
    real,
    string,
    boolean
};

class variant_value
{
    storage_value_t m_storage;
    variant_type_t  m_variant_type;
public:

    template<typename T>
    variant_value(const T& value);

    template<typename T>
    void set(const T& value);

    template<typename T>
    T get(const T& default_value = {}) const;

    variant_type_t type() const;
    void set_type(variant_type_t variant_type);
};

void test();

}

#endif // VARIANT_VALUE_H
