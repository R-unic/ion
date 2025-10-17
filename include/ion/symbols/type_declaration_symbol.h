#pragma once
#include <string>

#include "type_symbol.h"
#include "ion/ast/node.h"
#include "ion/ast/statements/named_declaration.h"

struct TypeDeclarationSymbol final : DeclarationSymbol
{
    type_ptr_t type;

    explicit TypeDeclarationSymbol(std::string name, type_ptr_t type, const NamedDeclaration* declaration)
        : DeclarationSymbol(std::move(name), declaration, std::move(type)),
          type(std::move(*Symbol::type))
    {
    }

    [[nodiscard]] std::string to_string() const override
    {
        return "TypeDeclarationSymbol(" + name + ", " + type->to_string() + ')' + declaring_symbol_to_string();
    }

    [[nodiscard]] bool is_type_declaration_symbol() const override
    {
        return true;
    }
};

using type_declaration_symbol_ptr_t = std::shared_ptr<TypeDeclarationSymbol>;