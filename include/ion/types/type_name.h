#pragma once
#include <optional>
#include <string>

#include "type.h"

static std::string get_last_segment(const std::string& s)
{
    const auto pos = s.rfind("::");
    return pos == std::string::npos ? s : s.substr(pos + 2);
}

struct TypeName final : Type
{
    std::string name;

    explicit TypeName(std::string name)
        : name(std::move(name))
    {
    }

    [[nodiscard]] bool is_same(const type_ptr_t& other) const override
    {
        CAST_CHECK(type_name, TypeName);
        return name == type_name->name;
    }

    TYPE_OVERRIDES(type_name, TypeName);

    [[nodiscard]] std::string to_string() const override
    {
        return name;
    }
};