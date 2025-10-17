#pragma once
#include <algorithm>

#include "ion/token.h"
#include "ion/ast/node.h"

class InterfaceField final : public NamedDeclaration
{
public:
    std::optional<Token> const_keyword;
    Token colon_token;
    type_ref_ptr_t type;

    explicit InterfaceField(std::optional<Token> const_keyword, Token name, Token colon_token, type_ref_ptr_t type)
        : NamedDeclaration(std::move(name)),
          const_keyword(std::move(const_keyword)),
          colon_token(std::move(colon_token)),
          type(std::move(type))
    {
    }

    static statement_ptr_t create(std::optional<Token> const_keyword, Token name, Token colon_token, type_ref_ptr_t type)
    {
        return std::make_unique<InterfaceField>(std::move(const_keyword), std::move(name), std::move(colon_token), std::move(type));
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_interface_field(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return name;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return type->get_last_token();
    }

    [[nodiscard]] std::string get_text() const override
    {
        return name.get_text() + ": " + type->get_text();
    }
};