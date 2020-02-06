#include "variant_value.h"
#include <cstring>

namespace base
{

template <typename Tin>
static void store_value(storage_value_t& storage
                        , const Tin& value)
{

    storage.resize(sizeof(Tin));
    reinterpret_cast<Tin&>(*storage.data()) = value;
}

static void store_value(storage_value_t& storage
                        , const string_type_t& value)
{
    storage.resize(value.size());
    std::memcpy(storage.data()
                , value.data()
                , value.size());
}

template<typename Tout>
static Tout load_value(const storage_value_t& storage, const Tout& default_value = {})
{
    if (storage.size() == sizeof(Tout))
    {
        return reinterpret_cast<const Tout&>(*storage.data());
    }

    return default_value;
}

static string_type_t load_value(const storage_value_t& storage, const string_type_t& default_value = {})
{
    string_type_t string_value = default_value;

    if (!storage.empty())
    {
        string_value.resize(storage.size());
        std::memcpy(&string_value[0]
                    , storage.data()
                    , storage.size());
    }

    return string_value;
}

template<typename Tin>
variant_type_t get_type();

template<>
variant_type_t get_type<numeric_type_t>()
{
    return variant_type_t::numeric;
}

template<>
variant_type_t get_type<real_type_t>()
{
    return variant_type_t::real;
}

template<>
variant_type_t get_type<string_type_t>()
{
    return variant_type_t::string;
}

template<>
variant_type_t get_type<boolean_type_t>()
{
    return variant_type_t::boolean;
}

template<typename Tout>
Tout from_string(const string_type_t& string_value
                           , const Tout& default_value = {});

template<>
string_type_t from_string(const string_type_t& string_value
                           , const string_type_t& default_value)
{
    return string_value;
}

template<>
numeric_type_t from_string(const string_type_t& string_value
                           , const numeric_type_t& default_value)
{
    try { return std::stoll(string_value); } catch(...) {}
    return default_value;
}

template<>
real_type_t from_string(const string_type_t& string_value
                           , const real_type_t& default_value)
{
    try { return std::stod(string_value); } catch(...) {}
    return default_value;
}

template<>
boolean_type_t from_string(const string_type_t& string_value
                           , const boolean_type_t& default_value)
{
    try { return std::stod(string_value) != 0; } catch(...) {}
    return default_value;
}

template<typename Tin, typename Tout>
static Tout convert_from(const Tin& value
                       , const Tout& default_value = {})
{
    return static_cast<Tout>(value);
}

template<typename Tin>
static string_type_t convert_from(const Tin& value
                                  , const string_type_t& default_value = {})
{
    return std::to_string(value);
}


template<typename Tout, typename Tin>
static Tout convert(const Tin& value
                    , const Tout& default_value = {})
{
    return convert_from(value
                        , default_value);
}

template<typename Tout>
static Tout convert(const string_type_t& value
                    , const Tout& default_value = {})
{
    return from_string(value
                       , default_value);
}


template<typename Tin>
void put_to_storage(storage_value_t& storage
                    , variant_type_t variant_type
                    , const Tin& value)
{
    switch (variant_type)
    {
        case variant_type_t::numeric:
            store_value(storage
                        , convert<numeric_type_t>(value));
        break;
        case variant_type_t::real:
            store_value(storage
                        , convert<real_type_t>(value));
        break;
        case variant_type_t::string:
            store_value(storage
                        , convert<string_type_t>(value));
        break;
        case variant_type_t::boolean:
            store_value(storage
                        , convert<boolean_type_t>(value));
        break;
    }
}

template<typename Tout>
Tout fetch_from_storage(const storage_value_t& storage
                    , variant_type_t variant_type
                        , const Tout& default_value = {})
{
    switch (variant_type)
    {
        case variant_type_t::numeric:
            return convert<Tout>(load_value<numeric_type_t>(storage)
                                 , default_value);
        break;
        case variant_type_t::real:
            return convert<Tout>(load_value<real_type_t>(storage)
                                 , default_value);
        break;
        case variant_type_t::string:
            return convert<Tout>(load_value(storage, std::string())
                                 , default_value);
        break;
        case variant_type_t::boolean:
            return convert<Tout>(load_value<boolean_type_t>(storage)
                                , default_value);
        break;
    }
}

/*
template<typename Tout>
Tout convert(const storage_value_t& storage
             , variant_type_t variant_type
             , const Tout& default_value = {})
{
    switch(variant_type)
    {
        case variant_type_t::numeric:
            return static_cast<Tout>(load_value<numeric_type_t>(storage));
        break;
        case variant_type_t::real:
            return static_cast<Tout>(load_value<real_type_t>(storage));
        break;
        case variant_type_t::string:
            return from_string(load_value<string_type_t>(storage)
                               , default_value);
        break;
        case variant_type_t::boolean:
            return static_cast<Tout>(load_value<boolean_type_t>(storage));
        break;
    }

    return default_value;
}

template<>
string_type_t convert(const storage_value_t& storage
             , variant_type_t variant_type
             , const string_type_t& default_value = {})
{
    switch(variant_type)
    {
        case variant_type_t::numeric:
            return std::to_string(load_value<numeric_type_t>(storage));
        break;
        case variant_type_t::real:
            return std::to_string(load_value<real_type_t>(storage));
        break;
        case variant_type_t::string:
            return from_string(load_value<string_type_t>(storage), default_value);
        break;
        case variant_type_t::boolean:
            return std::to_string(load_value<boolean_type_t>(storage));
        break;
    }

    return default_value;
}
*/

//-------------------------------------------------------------------------------------------------

template<>
variant_value::variant_value(const numeric_type_t &value)
    : m_variant_type(variant_type_t::numeric)
{
    store_value(m_storage
                , value);
}

template<>
variant_value::variant_value(const real_type_t &value)
    : m_variant_type(variant_type_t::real)
{
    store_value(m_storage
                , value);
}

template<>
variant_value::variant_value(const string_type_t &value)
    : m_variant_type(variant_type_t::string)
{
    store_value(m_storage
                , value);
}

template<>
variant_value::variant_value(const boolean_type_t &value)
    : m_variant_type(variant_type_t::boolean)
{
    store_value(m_storage
                , value);
}

template<>
void variant_value::set(const numeric_type_t &value)
{
    put_to_storage(m_storage
                   , m_variant_type
                   , value);
}

template<>
void variant_value::set(const real_type_t &value)
{
    put_to_storage(m_storage
                   , m_variant_type
                   , value);
}

template<>
void variant_value::set(const string_type_t &value)
{
    put_to_storage(m_storage
                   , m_variant_type
                   , value);
}

template<>
void variant_value::set(const boolean_type_t &value)
{
    put_to_storage(m_storage
                   , m_variant_type
                   , value);
}

template<>
numeric_type_t variant_value::get(const numeric_type_t& value) const
{
    return fetch_from_storage(m_storage
                              , m_variant_type
                              , value);
}

template<>
real_type_t variant_value::get(const real_type_t& value) const
{
    return fetch_from_storage(m_storage
                              , m_variant_type
                              , value);
}

template<>
string_type_t variant_value::get(const string_type_t& value) const
{
    return fetch_from_storage(m_storage
                              , m_variant_type
                              , value);
}


template<>
boolean_type_t variant_value::get(const boolean_type_t& value) const
{
    return fetch_from_storage(m_storage
                              , m_variant_type
                              , value);
}


variant_type_t variant_value::type() const
{
    return m_variant_type;
}

void variant_value::set_type(variant_type_t variant_type)
{
    if (m_variant_type != variant_type)
    {

    }
}

void test()
{
    variant_value value(std::string("45.67"));

    auto a = value.get<numeric_type_t>();
    auto b = value.get<real_type_t>();
    auto c = value.get<string_type_t>();
    auto d = value.get<boolean_type_t>();

    return;
}


}
