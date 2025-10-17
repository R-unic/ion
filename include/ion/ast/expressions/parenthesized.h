#pragma once
#include "ion/token.h"
#include "ion/ast/node.h"

class Parenthesized final : public Expression
{
public:
    Token l_paren, r_paren;
    expression_ptr_t expression;

    explicit Parenthesized(Token l_paren, Token r_paren, expression_ptr_t expression)
        : l_paren(std::move(l_paren)),
          r_paren(std::move(r_paren)),
          expression(std::move(expression))
    {
    }

    static expression_ptr_t create(Token l_paren, Token r_paren, expression_ptr_t expression)
    {
        return std::make_unique<Parenthesized>(std::move(l_paren), std::move(r_paren), std::move(expression));
    }

    void accept(ExpressionVisitor<void>& visitor) override
    {
        return visitor.visit_parenthesized(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return l_paren;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return r_paren;
    }

    [[nodiscard]] std::string get_text() const override
    {
        return '(' + expression->get_text() + ')';
    }

    [[nodiscard]] bool is_parenthesized() const override
    {
        return true;
    }
};