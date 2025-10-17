#pragma once
#include "ion/token.h"
#include "ion/ast/node.h"

class ElementAccess final : public Expression
{
public:
    Token l_bracket, r_bracket;
    expression_ptr_t expression, index_expression;

    explicit ElementAccess(Token l_bracket, Token r_bracket, expression_ptr_t expression,
                           expression_ptr_t index_expression)
        : l_bracket(std::move(l_bracket)),
          r_bracket(std::move(r_bracket)),
          expression(std::move(expression)),
          index_expression(std::move(index_expression))
    {
    }

    static expression_ptr_t create(Token l_bracket, Token r_bracket, expression_ptr_t expression,
                                   expression_ptr_t index_expression)
    {
        return std::make_unique<ElementAccess>(std::move(l_bracket), std::move(r_bracket), std::move(expression),
                                               std::move(index_expression));
    }

    void accept(ExpressionVisitor<void>& visitor) override
    {
        return visitor.visit_element_access(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return expression->get_first_token();
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return r_bracket;
    }

    [[nodiscard]] std::string get_text() const override
    {
        return expression->get_text() + '[' + index_expression->get_text() + ']';
    }

    [[nodiscard]] bool is_assignment_target() const override
    {
        return true;
    }
};