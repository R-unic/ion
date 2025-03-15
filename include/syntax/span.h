#pragma once
#include "location.h"

class span
{
public:
    location start;
    location end;

    span(const location& start, const location& end);
};

inline std::string to_string(span span)
{
    return std::format("{} - {}", span.start, span.end);
}

template <>
struct std::formatter<span> : std::formatter<std::string_view> {
    auto format(const span& span, std::format_context& ctx) const {
        return std::formatter<std::string_view>::format(to_string(span), ctx);
    }
};
