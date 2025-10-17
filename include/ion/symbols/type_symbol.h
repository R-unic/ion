#pragma once
#include <string>

#include "named_symbol.h"

struct TypeSymbol final : NamedSymbol
{
    type_ptr_t type;

    explicit TypeSymbol(std::string name, type_ptr_t type)
        : NamedSymbol(std::move(name), std::move(type)),
          type(std::move(*Symbol::type))
    {
    }

    [[nodiscard]] std::string to_string() const override
    {
        return "TypeSymbol(" + name + ", " + type->to_string() + ')' + declaring_symbol_to_string();
    }

    [[nodiscard]] bool is_type_symbol() const override
    {
        return true;
    }
};

using type_symbol_ptr_t = std::shared_ptr<TypeSymbol>;