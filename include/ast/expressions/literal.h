#pragma once
#include <optional>
#include <variant>
#include <string>

#include "ast/node.h"

using literal_value_t = std::optional<std::variant<double, bool, std::string>>;

class Literal final : public Expression
{
public:
    literal_value_t value;

    explicit Literal(literal_value_t value)
        : value(std::move(value))
    {
    }
    
    static expression_ptr_t create(const literal_value_t& value)
    {
        return std::make_unique<Literal>(value);
    }
    
    void accept(ExpressionVisitor<void>& visitor) override
    {
        return visitor.visit_literal(*this);
    }
};
