#pragma once
#include <string>

#include "ion/token.h"
#include "ion/ast/node.h"

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

    [[nodiscard]] Token get_first_token() const override
    {
        return name;
    }

    [[nodiscard]] std::string get_text() const override
    {
        return name.get_text();
    }

    [[nodiscard]] bool is_name_of_target() const override
    {
        return true;
    }

    [[nodiscard]] bool is_assignment_target() const override
    {
        return true;
    }
};