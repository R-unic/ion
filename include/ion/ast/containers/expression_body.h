#pragma once
#include "ion/token.h"
#include "ion/ast/node.h"

struct ExpressionBody final : SyntaxNode
{
    Token long_arrow;
    expression_ptr_t expression;

    ExpressionBody(Token long_arrow, expression_ptr_t expression)
        : long_arrow(std::move(long_arrow)),
          expression(std::move(expression))
    {
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return long_arrow;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return expression->get_last_token();
    }

    [[nodiscard]] std::string get_text() const override
    {
        return " -> " + expression->get_text();
    }
};