#pragma once
#include "ion/token.h"
#include "ion/ast/node.h"

class Await final : public Expression
{
public:
    Token keyword;
    expression_ptr_t expression;

    explicit Await(Token keyword, expression_ptr_t expression)
        : keyword(std::move(keyword)),
          expression(std::move(expression))
    {
    }

    static expression_ptr_t create(Token keyword, expression_ptr_t expression)
    {
        return std::make_unique<Await>(std::move(keyword), std::move(expression));
    }

    void accept(ExpressionVisitor<void>& visitor) override
    {
        return visitor.visit_await(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return keyword;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return expression->get_last_token();
    }

    [[nodiscard]] std::string get_text() const override
    {
        return "await " + expression->get_text();
    }
};