#pragma once
#include <optional>
#include <variant>
#include <string>

#include "ast/node.h"

class Identifier final : public Expression
{
public:
    std::string name;

    explicit Identifier(std::string name)
        : name(std::move(name))
    {
    }
    
    static expression_ptr_t create(const std::string& value)
    {
        return std::make_unique<Identifier>(value);
    }
    
    void accept(ExpressionVisitor<void>& visitor) override
    {
        return visitor.visit_identifier(*this);
    }
};
