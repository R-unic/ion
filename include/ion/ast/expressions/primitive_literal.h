#pragma once
#include <optional>
#include <variant>
#include <string>

#include "ion/token.h"
#include "ion/ast/node.h"
#include "ion/utility/basic.h"

class PrimitiveLiteral final : public Expression
{
public:
    Token token;
    std::optional<primitive_value_t> value;

    explicit PrimitiveLiteral(Token token, std::optional<primitive_value_t> value)
        : token(std::move(token)),
          value(std::move(value))
    {
    }

    static expression_ptr_t create(Token token, std::optional<primitive_value_t> value)
    {
        return std::make_unique<PrimitiveLiteral>(std::move(token), std::move(value));
    }

    void accept(ExpressionVisitor<void>& visitor) override
    {
        return visitor.visit_primitive_literal(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return token;
    }

    [[nodiscard]] std::string get_text() const override
    {
        return primitive_to_string(value);
    }

    [[nodiscard]] bool is_null_literal() const override
    {
        return !value.has_value();
    }

    [[nodiscard]] bool is_primitive_literal() const override
    {
        return true;
    }

    [[nodiscard]] bool is_literal() const override
    {
        return true;
    }
};