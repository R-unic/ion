#pragma once
#include <algorithm>

#include "ast/node.h"

class Return final : public Statement
{
public:
    Token keyword;
    std::optional<expression_ptr_t> expression;
    
    explicit Return(Token keyword, std::optional<expression_ptr_t> expression)
        : keyword(std::move(keyword)), expression(std::move(expression))
    {
    }
    
    static statement_ptr_t create(Token keyword, std::optional<expression_ptr_t> expression)
    {
        return std::make_unique<Return>(std::move(keyword), std::move(expression));
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_return(*this);
    }
};