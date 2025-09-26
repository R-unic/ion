#pragma once
#include <algorithm>

#include "ast/node.h"

class Break final : public Statement
{
public:
    Token keyword;
    
    explicit Break(Token keyword) : keyword(std::move(keyword))
    {
    }
    
    static statement_ptr_t create(Token keyword)
    {
        return std::make_unique<Break>(std::move(keyword));
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_break(*this);
    }

    [[nodiscard]] FileSpan get_span() const override
    {
        return keyword.span;
    }
    
    [[nodiscard]] std::string get_text() const override
    {
        return keyword.get_text();
    }
};