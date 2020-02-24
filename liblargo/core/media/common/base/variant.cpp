#include "variant.h"
#include <cstring>
#include <sstream>
#include <sstream>
#include <limits>

namespace base
{

using string = std::string;
using int8 = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;
using int64 = std::int64_t;
using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;
using long_double = long double;
using _double = double;


#define DEFINE_BLOCK(block_name) \
    block_name(int8) \
    block_name(int16) \
    block_name(int32) \
    block_name(int64) \
    block_name(uint8) \
    block_name(uint16) \
    block_name(uint32) \
    block_name(uint64) \
    block_name(float) \
    block_name(double) \
    block_name(long_double) \
    block_name(string) \
    block_name(bool) \

#define DEFINE_BLOCK_ARGS(block_name, ...) \
    block_name(int8, __VA_ARGS__) \
    block_name(int16, __VA_ARGS__) \
    block_name(int32, __VA_ARGS__) \
    block_name(int64, __VA_ARGS__) \
    block_name(uint8, __VA_ARGS__) \
    block_name(uint16, __VA_ARGS__) \
    block_name(uint32, __VA_ARGS__) \
    block_name(uint64, __VA_ARGS__) \
    block_name(float, __VA_ARGS__) \
    block_name(double, __VA_ARGS__) \
    block_name(long_double, __VA_ARGS__) \
    block_name(string, __VA_ARGS__) \
    block_name(bool, __VA_ARGS__) \


template<typename T>
T max_value()
{
    return std::numeric_limits<T>::max();
}

template<typename T>
T min_value()
{
    return std::numeric_limits<T>::min();
}

template<typename T>
T from_string(const std::string& string_value
              , const T& default_value = {})
{
    T result_value = {};
    std::stringstream ss;

    ss << string_value;
    ss >> result_value;

    return ss.fail()
            ? default_value
            : result_value;
}

template<typename T>
std::string to_string(const T& value
                      , const std::string& default_value = {})
{
    std::string string_value = {};
    std::stringstream ss;

    ss << value;
    ss >> string_value;

    return ss.fail()
            ? default_value
            : string_value;
}

template<typename Tin, typename Tout>
struct converter
{
    static Tout convert(const Tin& value
                        , const Tout& default_value= {})
    {
        return static_cast<Tout>(value);
    }
};

template<typename Tin>
struct converter<Tin, std::string>
{
    static std::string convert(const Tin& value
                               , const std::string& default_value = {})
    {
        return to_string(value
                         , default_value);
    }
};

template<typename Tout>
struct converter<std::string, Tout>
{
    static Tout convert(const std::string& value
                        , const Tout& default_value= {})
    {
        return from_string(value
                         , default_value);
    }
};

template<>
struct converter<std::string, std::string>
{
    static std::string convert(const std::string& value
                        , const std::string& default_value= {})
    {
        return value;
    }
};

template<typename T>
constexpr bool is_container()
{
    return false;
}

template<>
constexpr bool is_container<std::string>()
{
    return true;
}

template<typename T, bool C = is_container<T>()>
struct storage_serializer_t
{
    storage_serializer_t(storage_value_t& storage_value);
    void set(const T& value);
};

template<typename T>
struct storage_serializer_t<T, false>
{
    static constexpr auto value_size = sizeof(T);

    storage_value_t&    m_storage_value;

    storage_serializer_t(storage_value_t& storage_value)
        : m_storage_value(storage_value)
    {

    }

    void set(const T& value)
    {
        m_storage_value.resize(value_size);
        std::memcpy(m_storage_value.data()
                    , &value
                    , value_size);
    }
};

template<typename T>
struct storage_serializer_t<T, true>
{
    static const auto value_size = sizeof(typename T::value_type);

    storage_value_t&    m_storage_value;

    storage_serializer_t(storage_value_t& storage_value)
        : m_storage_value(storage_value)
    {

    }

    void set(const T& value)
    {
        m_storage_value.resize(value.size() * value_size);
        std::memcpy(m_storage_value.data()
                    , value.data()
                    , m_storage_value.size());
    }
};

template<typename T, bool C = is_container<T>()>
struct storage_deserializer_t
{
    storage_deserializer_t(const storage_value_t& storage_value);
    T get(const T& default_value = {});
};

template<typename T>
struct storage_deserializer_t<T, false>
{
    static constexpr auto value_size = sizeof(T);

    const storage_value_t&  m_storage_value;

    storage_deserializer_t(const storage_value_t& storage_value)
        : m_storage_value(storage_value)
    {

    }

    T get(const T& default_value = {})
    {
        if (m_storage_value.size() == value_size)
        {
            return reinterpret_cast<const T&>(*m_storage_value.data());
        }

        return default_value;
    }
};

template<typename T>
struct storage_deserializer_t<T, true>
{
    static constexpr auto value_size = sizeof(typename T::value_type);

    const storage_value_t&  m_storage_value;

    storage_deserializer_t(const storage_value_t& storage_value)
        : m_storage_value(storage_value)
    {

    }

    T get(const T& default_value = {})
    {
        auto result_value = default_value;

        if (!m_storage_value.empty())
        {
            result_value.resize(m_storage_value.size() / value_size);
            std::memcpy(&result_value[0]
                        , m_storage_value.data()
                        , m_storage_value.size());
        }

        return result_value;
    }
};

template<typename Tout>
Tout get_storage_value(const storage_value_t& strorage
                       , variant_type_t store_type
                       , const Tout& default_value={})
{
    switch(store_type)
    {
        #define CASE_STORAGE_VALUE(type) \
        case variant_type_t::vt_##type: \
        { \
            return converter<type, Tout>::convert(storage_deserializer_t<type>(strorage).get() \
                                                  , default_value); \
        } \
        break;

        DEFINE_BLOCK(CASE_STORAGE_VALUE)
    }

    return default_value;
}


//-------------------------------------------------------------------------------------

#define DEFINE_VARIANT_CONSTRUCTOR(type) \
template<> \
variant::variant(const type& value) \
    : m_variant_type(variant_type_t::vt_##type) \
{ \
    set<type>(value); \
}

#define DEFINE_VARIANT_SETTER(type) \
template<> \
void variant::set(const type& value) \
{ \
    m_variant_type = variant_type_t::vt_##type; \
    storage_serializer_t<type> serializer(m_storage); \
    serializer.set(value); \
} \
template<> \
variant& variant::operator=(const type& value) \
{ \
    set(value); \
    return *this; \
}

#define DEFINE_VARIANT_GETTER(type) \
template<> \
type variant::get(const type& default_value) const \
{ \
    return get_storage_value<type>(m_storage \
                                   , m_variant_type \
                                   , default_value); \
} \
template<> \
variant::operator type() const \
{ \
    return get<type>(); \
}

#define DEFINE_VARIANT_TYPE(type) \
    DEFINE_VARIANT_SETTER(type) \
    DEFINE_VARIANT_GETTER(type) \
    DEFINE_VARIANT_CONSTRUCTOR(type)

DEFINE_BLOCK(DEFINE_VARIANT_TYPE)

variant::variant()
    : m_variant_type(variant_type_t::vt_unknown)
{

}

variant::variant(const char *value)
    : variant(std::string(value))
{

}

void variant::set(const char *value)
{
    set<std::string>(value);
}

#define CASE_COMPARE(type, op) \
case variant_type_t::vt_##type: \
{ \
    return get<type>() op variant.get<type>(max_value<type>()); \
} \
break;

#define DEFINE_COMPARE_OPERATOR(op) \
bool variant::operator op (const variant &variant) const \
{ \
    switch(m_variant_type) \
    { \
        DEFINE_BLOCK_ARGS(CASE_COMPARE, op) \
    } \
    return false; \
}

DEFINE_COMPARE_OPERATOR(==)
DEFINE_COMPARE_OPERATOR(!=)
DEFINE_COMPARE_OPERATOR(>)
DEFINE_COMPARE_OPERATOR(>=)
DEFINE_COMPARE_OPERATOR(<)
DEFINE_COMPARE_OPERATOR(<=)

void variant::set_type(variant_type_t variant_type)
{
    if (variant_type != m_variant_type)
    {
        switch(variant_type)
        {
            #define CASE_SET_VALUE(type) \
            case variant_type_t::vt_##type: \
            { \
                set(get<type>()); \
            } \
            break;

            DEFINE_BLOCK(CASE_SET_VALUE);
        }
    }
}

variant_type_t variant::type() const
{
    return m_variant_type;
}

bool variant::empty() const
{
    return m_variant_type == variant_type_t::vt_unknown
            || m_storage.empty();
}


void test()
{

    variant var1(45);
    variant var2("vds");

    bool f1 = var1 > var2;
    bool f2 = var2 > var1;
    bool f3 = var1 < var2;
    bool f4 = var2 < var1;
    bool f5 = var1 == var2;

    auto r = min_value<std::string>();

    return;
    /*
    storage_value_t storage1;
    storage_value_t storage2;
    storage_serializer_t<int> serializer1(storage1);
    storage_serializer_t<std::string> serializer2(storage2);

    storage_deserializer_t<int> deserializer1(storage1);
    storage_deserializer_t<std::string> deserializer2(storage2);



    serializer1.set(1);
    serializer2.set("vasily");

    auto v1 = deserializer1.get();
    auto v11 = get_storage_value<std::string>(storage1
                                              , variant_type_t::vt_int32
                                              , "");
    auto v2 = deserializer2.get();

    double dbl = 5;
    bool fl = false;
    int n = 643;

    std::stringstream ss;

    ss << "vcs";

    ss >> n;


    auto is_fail = ss.fail();
    auto is_bad = ss.bad();

    int n_value = -56;

    auto s_value = converter<int, std::string>::convert(n_value);

    auto m_value = converter<std::string, std::string>::convert(s_value);
    n_value = converter<std::string, int>::convert(s_value);

    return;*/
}

}
