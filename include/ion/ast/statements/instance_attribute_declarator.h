#pragma once
#include <algorithm>

#include "ion/token.h"
#include "ion/ast/node.h"

class InstanceAttributeDeclarator final : public InstanceDeclarator
{
public:
    Token at_token, colon_token;
    expression_ptr_t value;

    explicit InstanceAttributeDeclarator(Token at_token, Token name, Token colon_token, expression_ptr_t value)
        : InstanceDeclarator(std::move(name)),
          at_token(std::move(at_token)),
          colon_token(std::move(colon_token)),
          value(std::move(value))
    {
    }

    static statement_ptr_t create(Token at_token, Token name, Token colon_token, expression_ptr_t value)
    {
        return std::make_unique<
            InstanceAttributeDeclarator>(std::move(at_token), std::move(name), std::move(colon_token), std::move(value));
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_instance_attribute_declarator(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return at_token;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return value->get_last_token();
    }

    [[nodiscard]] std::string get_text() const override
    {
        return '@' + name.get_text() + ": " + value->get_text();
    }
};