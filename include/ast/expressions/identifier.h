#pragma once
#include <string>

#include "ast/node.h"

class Identifier final : public Expression
{
public:
    Token name;

    explicit Identifier(Token name)
        : name(std::move(name))
    {
    }
    
    static expression_ptr_t create(Token value)
    {
        return std::make_unique<Identifier>(std::move(value));
    }
    
    void accept(ExpressionVisitor<void>& visitor) override
    {
        return visitor.visit_identifier(*this);
    }

    [[nodiscard]] FileSpan get_span() const override
    {
        return name.span;
    }
    
    [[nodiscard]] std::string get_text() const override
    {
       return name.get_text();
    }
};
