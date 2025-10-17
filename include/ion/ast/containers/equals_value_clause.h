#pragma once
#include "ion/token.h"
#include "ion/ast/node.h"

struct EqualsValueClause final : SyntaxNode
{
    Token equals_token;
    expression_ptr_t value;

    explicit EqualsValueClause(Token equals_token, expression_ptr_t value)
        : equals_token(std::move(equals_token)),
          value(std::move(value))
    {
    }

    EqualsValueClause(const EqualsValueClause&) = delete;
    EqualsValueClause& operator=(const EqualsValueClause&) = delete;

    [[nodiscard]] Token get_first_token() const override
    {
        return equals_token;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return value->get_last_token();
    }

    [[nodiscard]] std::string get_text() const override
    {
        return " = " + value->get_text();
    }
};