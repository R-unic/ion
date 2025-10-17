#pragma once
#include <string>

#include "symbol.h"

struct NamedSymbol : Symbol
{
    std::string name;

    explicit NamedSymbol(std::string name, std::optional<type_ptr_t> type = std::nullopt)
        : Symbol(std::move(type)),
          name(std::move(name))
    {
    }

    [[nodiscard]] std::string to_string() const override
    {
        return "NamedSymbol(" + name + (type.has_value() ? ", " + type.value()->to_string() : "") + ')' + declaring_symbol_to_string();
    }

    [[nodiscard]] bool is_named_symbol() const override
    {
        return true;
    }
};

using named_symbol_ptr_t = std::shared_ptr<NamedSymbol>;