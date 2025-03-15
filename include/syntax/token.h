#pragma once
#include <string>
#include <variant>
#include <format>

#include "span.h"
#include "syntax_kind.h"

class token
{
public:
    using value_t = std::variant<std::monostate, int, double, bool, std::string>;
    
    syntax_kind kind;
    span span;
    std::string text;
    value_t value = nullptr;

    token(syntax_kind kind, ::span span, const std::string& text);
    token(syntax_kind kind, ::span span, const std::string& text, value_t value);
    token(syntax_kind kind, ::span span, const std::string& text, std::string value);
    token(syntax_kind kind, ::span span, const std::string& text, bool value);
    token(syntax_kind kind, ::span span, const std::string& text, int value);
    token(syntax_kind kind, ::span span, const std::string& text, float value);
};

inline std::string to_string(const token::value_t& value)
{
    try {
        const auto double_value = std::get<double>(value);
        return std::to_string(double_value);
    } catch (const std::bad_variant_access&) {}

    try {
        const auto int_value = std::get<int>(value);
        return std::to_string(int_value);
    } catch (const std::bad_variant_access&) {}

    try {
        const auto bool_value = std::get<bool>(value);
        return std::to_string(bool_value);
    } catch (const std::bad_variant_access&) {}

    try {
        auto string_value = std::get<std::string>(value);
        return string_value;
    } catch (const std::bad_variant_access&) {}

    return "null";
}

template <>
struct std::formatter<token::value_t> : std::formatter<std::string_view> {
    auto format(const token::value_t& value, std::format_context& ctx) const {
        return std::formatter<std::string_view>::format(::to_string(value), ctx);
    }
};

inline std::string to_string(token token)
{
    return std::format("{}: {} | {}    {}", token.kind, token.text, to_string(token.value), token.span);
}

template <>
struct std::formatter<token> : std::formatter<std::string_view> {
    auto format(const token& token, std::format_context& ctx) const {
        return std::formatter<std::string_view>::format(to_string(token), ctx);
    }
};
