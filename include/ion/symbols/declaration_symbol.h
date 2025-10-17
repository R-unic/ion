#pragma once
#include <string>

#include "named_symbol.h"
#include "ion/ast/node.h"

struct DeclarationSymbol : NamedSymbol
{
    const NamedDeclaration* declaration;

    explicit DeclarationSymbol(std::string name, const NamedDeclaration* declaration, std::optional<type_ptr_t> type = std::nullopt)
        : NamedSymbol(std::move(name), std::move(type)),
          declaration(declaration)
    {
    }

    [[nodiscard]] std::string to_string() const override
    {
        return "DeclarationSymbol(" + name + (type.has_value() ? ", " + type.value()->to_string() : "") + ')' +
               declaring_symbol_to_string();
    }

    [[nodiscard]] bool is_declaration_symbol() const override
    {
        return true;
    }
};

using declaration_symbol_ptr_t = std::shared_ptr<DeclarationSymbol>;