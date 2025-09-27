#include <chrono>
#include <iostream>

#include "ion/logger.h"

static std::tm get_local_time()
{
    const auto now = std::chrono::system_clock::now();
    const auto now_time = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm {};
#ifdef _WIN32
    localtime_s(&local_tm, &now_time); // Windows secure version
#else
    localtime_r(&now_time, &local_tm); // POSIX thread-safe version
#endif

    return local_tm;
}

static void log(const std::string& tag, const std::string& message)
{
    const auto local_time = get_local_time();
    std::cout << "[" << std::put_time(&local_time, "%H:%M:%S") << " " << tag << "] " << message << '\n';
}

namespace logger
{
    void info(const std::string& message)
    {
        log("INFO", message);
    }

    void warn(const std::string& message)
    {
        log("WARN", message);
    }

    void debug(const std::string& message)
    {
        log("DBG", message);
    }

    void verbose(const std::string& message)
    {
        log("VERB", message);
    }
}