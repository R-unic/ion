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

    [[nodiscard]] type_ptr_t as_shared() const override
    {
        return std::make_shared<TypeName>(*this);
    }

    [[nodiscard]] std::string to_string() const override
    {
        return name;
    }
};