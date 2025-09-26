#pragma once
#include "token.h"
#include "ast/node.h"

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

    [[nodiscard]] FileSpan get_span() const override
    {
        return create_span(l_paren.span.start, r_paren.span.end);
    }
    
    [[nodiscard]] std::string get_text() const override
    {
        return l_paren.get_text() + expression->get_text() + r_paren.get_text();
    }
};