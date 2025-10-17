#pragma once
#include <string>

namespace logger
{
    void info(const std::string&);
    void warn(const std::string&);
    [[noreturn]] void error(const std::string& message, uint8_t code = 1);
    void debug(const std::string&);
    void verbose(const std::string&);
}