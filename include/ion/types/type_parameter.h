#pragma once
#include <optional>
#include <string>

#include "type.h"

struct TypeParameter final : Type
{
    std::string name;
    std::optional<type_ptr_t> base_type, default_type;

    explicit TypeParameter(std::string name, std::optional<type_ptr_t> base_type, std::optional<type_ptr_t> default_type)
        : name(std::move(name)),
          base_type(std::move(base_type)),
          default_type(std::move(default_type))
    {
    }

    [[nodiscard]] type_ptr_t as_shared() const override
    {
        return std::make_shared<TypeParameter>(*this);
    }

    [[nodiscard]] std::string to_string() const override
    {
        const auto base_type_text = base_type.has_value() ? " : " + base_type.value()->to_string() : "";
        const auto default_type_text = default_type.has_value() ? " = " + default_type.value()->to_string() : "";
        return name + base_type_text + default_type_text;
    }
};