#pragma once
#include <string>

#include "ion/token.h"
#include "ion/ast/node.h"

class RangeLiteral final : public Expression
{
public:
    Token dot_dot_token;
    expression_ptr_t minimum, maximum;

    explicit RangeLiteral(expression_ptr_t minimum, Token dot_dot_token, expression_ptr_t maximum)
        : dot_dot_token(std::move(dot_dot_token)),
          minimum(std::move(minimum)),
          maximum(std::move(maximum))
    {
    }

    static expression_ptr_t create(expression_ptr_t minimum, Token dot_dot_token, expression_ptr_t maximum)
    {
        return std::make_unique<RangeLiteral>(std::move(minimum), std::move(dot_dot_token), std::move(maximum));
    }

    void accept(ExpressionVisitor<void>& visitor) override
    {
        return visitor.visit_range_literal(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return minimum->get_first_token();
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return maximum->get_last_token();
    }

    [[nodiscard]] std::string get_text() const override
    {
        const auto minimum_text = minimum->is_primitive_literal() ? minimum->get_text() : '(' + minimum->get_text() + ')';
        const auto maximum_text = maximum->is_primitive_literal() ? maximum->get_text() : '(' + maximum->get_text() + ')';
        return minimum_text + ".." + maximum_text;
    }

    [[nodiscard]] bool is_literal() const override
    {
        return true;
    }
};