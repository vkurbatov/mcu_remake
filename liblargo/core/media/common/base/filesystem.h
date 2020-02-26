#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>
#include <vector>

namespace filesystem
{

typedef std::vector<std::string> file_list_t;

file_list_t get_files(const std::string& dir_name);
std::string extract_file_extension(const std::string& file_name);

}

#endif // FILESYSTEM_H
