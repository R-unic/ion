#pragma once
#include <algorithm>

#include "ast/node.h"

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
};