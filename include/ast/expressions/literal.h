#pragma once
#include <optional>
#include <variant>
#include <string>

#include "token.h"
#include "ast/node.h"

using literal_value_t = std::optional<std::variant<double, bool, std::string>>;

class Literal final : public Expression
{
public:
    Token token;
    literal_value_t value;

    explicit Literal(Token token, literal_value_t value)
        : token(std::move(token)),
        value(std::move(value))
    {
    }
    
    static expression_ptr_t create(Token token, literal_value_t value)
    {
        return std::make_unique<Literal>(std::move(token), std::move(value));
    }
    
    void accept(ExpressionVisitor<void>& visitor) override
    {
        return visitor.visit_literal(*this);
    }

    [[nodiscard]] FileSpan get_span() const override
    {
        return token.span;
    }
    
    [[nodiscard]] std::string get_text() const override
    {
        if (!value.has_value())
            return "null";
        
        return std::visit([]<typename T>(const T& arg)
        {
            using type_t = std::decay_t<T>;
            if constexpr (std::is_same_v<type_t, double>)
                return std::to_string(arg);
            else if constexpr (std::is_same_v<type_t, std::string>)
                return '"' + arg + '"';
            else if constexpr (std::is_same_v<type_t, bool>)
                return std::string(arg ? "true" : "false");
        }, *value);
    }
};
