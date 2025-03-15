#pragma once
#include <format>
#include <string>

class location {
public:
    std::string file_name;
    size_t line;
    size_t column;
    size_t position;

    location(const std::string& file_name, size_t line, size_t column, size_t position);

    static location empty(const std::string& name = "?");
};

inline std::string to_string(location location)
{
    return std::format("{}:{}:{}", location.file_name, location.line, location.column);
}

template <>
struct std::formatter<location> : std::formatter<std::string_view> {
    auto format(const location& location, std::format_context& ctx) const {
        return std::formatter<std::string_view>::format(to_string(location), ctx);
    }
};
