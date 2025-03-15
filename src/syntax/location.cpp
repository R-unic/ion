#include <string>
#include "location.h"

location::location(const std::string& file_name, const size_t line, const size_t column, const size_t position)
    : file_name(file_name), line(line), column(column), position(position) {}

location location::empty(const std::string& name)
{
    return location(name, 1, 0, 0);
}