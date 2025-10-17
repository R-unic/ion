#pragma once
#include <algorithm>
#include <string>

#include "ion/file_location.h"
#include "ion/ast/node.h"

class ExpressionStatement final : public Statement
{
public:
    expression_ptr_t expression;

    explicit ExpressionStatement(expression_ptr_t expression)
        : expression(std::move(expression))
    {
    }

    static statement_ptr_t create(expression_ptr_t expression)
    {
        return std::make_unique<ExpressionStatement>(std::move(expression));
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_expression_statement(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return expression->get_first_token();
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return expression->get_last_token();
    }

    [[nodiscard]] std::string get_text() const override
    {
        return expression->get_text();
    }
};