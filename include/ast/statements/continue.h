#pragma once
#include <algorithm>

#include "ast/node.h"

class Continue final : public Statement
{
public:
    Token keyword;
    
    explicit Continue(Token keyword) : keyword(std::move(keyword))
    {
    }
    
    static statement_ptr_t create(Token keyword)
    {
        return std::make_unique<Continue>(std::move(keyword));
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_continue(*this);
    }
};