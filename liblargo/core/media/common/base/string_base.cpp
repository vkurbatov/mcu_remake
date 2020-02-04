#include "string_base.h"
#include <cstdarg>


namespace base
{

std::string hex_dump(const void *dump, std::size_t size)
{
    std::string hex_string;

    hex_string.reserve(size * 3 + 1);

    auto data = static_cast<const std::uint8_t*>(dump);

    char hex[4] = {};

    while (size-- > 0)
    {
        if (!hex_string.empty())
        {
            hex_string.append(" ");
        }

        std::sprintf(hex, "%02x", *data);
        hex_string.append(hex);

        data++;
    }

    return hex_string;
}

std::vector<std::string> split_string(const std::string &string
                                      , const std::string& delimiters)
{
    std::vector<std::string> split_list;

    std::string::size_type begin = 0;
    std::string::size_type end = std::string::npos;

    for (const auto& c : delimiters)
    {
        end = string.find(c, begin);

        if (end != std::string::npos)
        {
            split_list.push_back(string.substr(begin, end - begin));
            begin = end + 1;
        }
        else
        {
            split_list.push_back("");
        }
    }

    if (end != std::string::npos)
    {
        split_list.push_back(string.substr(begin, std::string::npos));
    }

    return std::move(split_list);
}

std::string format_string(const std::string &format, ...)
{
    char buffer[1024];

    va_list args;
    va_start(args, format);

    vsnprintf(buffer, sizeof(buffer), format.c_str(), args);

    va_end(args);

    return buffer;

}


}
