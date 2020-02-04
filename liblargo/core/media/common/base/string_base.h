#ifndef STRING_BASE_H
#define STRING_BASE_H

#include <string>
#include <vector>

namespace base
{

std::string hex_dump(const void *dump
                      , std::size_t size);

std::vector<std::string> split_string(const std::string& string
                                      , const std::vector<std::string>& delimiters);

std::string format_string(const std::string& format, ...);

}

#endif // STRING_BASE_H
