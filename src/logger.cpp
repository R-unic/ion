#include <chrono>
#include <iostream>

#include "ion/logger.h"
#include "ion/utility/basic.h"

static void log(const std::string& tag, const std::string& message)
{
    using namespace std::chrono;

    const auto time_stamp = color(std::format("{:%H:%M:%S}", floor<seconds>(system_clock::now())), Color::gray);
    std::cout << "[" << time_stamp << " " << tag << "] " << message << '\n';
}

namespace logger
{
    void info(const std::string& message)
    {
        log(color("INFO", Color::light_blue), message);
    }

    void warn(const std::string& message)
    {
        log(color("WARN", Color::yellow), message);
    }

    [[noreturn]] void error(const std::string& message, const uint8_t code)
    {
        log(color("ERR", Color::red), message);
        exit(code);
    }

    void debug(const std::string& message)
    {
        log(color("DBG", Color::purple), message);
    }

    void verbose(const std::string& message)
    {
        log("VERB", message);
    }
}