#pragma once
#include <variant>
#include <ranges>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#define DEFINE_IS_FN(name) \
    [[nodiscard]] virtual bool is_##name() const \
    { \
        return false; \
    }

#define TRUE_IS_FN(name) \
    [[nodiscard]] bool is_##name() const override \
    { \
        return true; \
    }

using primitive_value_t = std::variant<double, bool, std::string>;

inline double to_number(const std::string& s)
{
    if (s.starts_with("0x") || s.starts_with("0X"))
        return static_cast<double>(std::stoll(s, nullptr, 16));
    if (s.starts_with("0o") || s.starts_with("0O"))
        return static_cast<double>(std::stoll(s.substr(2), nullptr, 8));
    if (s.starts_with("0b") || s.starts_with("0B"))
        return static_cast<double>(std::stoll(s.substr(2), nullptr, 2));

    auto number = std::stod(s);
    if (s.ends_with("ms"))
        number /= 1000;
    else if (s.ends_with("m"))
        number *= 60;
    else if (s.ends_with("hz"))
        number /= 60;
    else if (s.ends_with("h"))
        number *= 3600;
    else if (s.ends_with("d"))
        number *= 86400;
    else if (s.ends_with("%"))
        number /= 100;

    return number;
}

inline std::string primitive_to_string(const std::optional<primitive_value_t>& value)
{
    if (!value.has_value())
        return "null";

    return std::visit([]<typename T>(const T& arg)
    {
        using type_t = std::decay_t<T>;
        if constexpr (std::is_same_v<type_t, double>)
            return std::to_string(arg);
        else if constexpr (std::is_same_v<type_t, bool>)
            return std::string(arg ? "true" : "false");
        else if constexpr (std::is_same_v<type_t, std::string>)
            return '"' + arg + '"';
    }, *value);
}

inline primitive_value_t primitive_from_string(const std::string& text)
{
    if (text == "true")
        return true;
    if (text == "false")
        return false;
    if (text.starts_with('"') || text.starts_with('\''))
        return text.substr(1, text.size() - 2);

    return to_number(text);
}

namespace Color
{
    constexpr auto reset = "\033[0m";
    constexpr auto black = "\033[30m";
    constexpr auto red = "\033[31m";
    constexpr auto yellow = "\033[33m";
    constexpr auto blue = "\033[34m";
    constexpr auto light_blue = "\033[38;5;12m";
    constexpr auto purple = "\033[35m";
    constexpr auto bold = "\033[1m";
    constexpr auto gray = "\033[38;5;243m";
    constexpr auto background_white = "\033[47m";
}

inline std::string color(const std::string& text, const char* color_code)
{
    return color_code + text + Color::reset;
}

template <typename R>
auto skip_first(R&& r)
{
    return std::forward<R>(r) | std::views::drop(1);
}

inline std::string join_by(const std::vector<std::string>& texts, const std::string& separator)
{
    std::ostringstream oss;
    auto first = true;
    for (auto& s : texts)
    {
        if (!first)
            oss << separator;

        oss << s;
        first = false;
    }

    return oss.str();
}

template <typename K, typename V>
std::unordered_map<V, K> inverse_map(const std::unordered_map<K, V>& forward_map)
{
    std::unordered_map<V, K> map;
    for (const auto& [lexeme, kind] : forward_map)
        map.emplace(kind, lexeme);

    return map;
}