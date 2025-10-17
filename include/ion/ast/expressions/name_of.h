#pragma once
#include "ion/token.h"
#include "ion/ast/node.h"

class NameOf final : public Expression
{
public:
    Token keyword, identifier;

    explicit NameOf(Token keyword, Token identifier)
        : keyword(std::move(keyword)),
          identifier(std::move(identifier))
    {
    }

    static expression_ptr_t create(Token keyword, Token identifier)
    {
        return std::make_unique<NameOf>(std::move(keyword), std::move(identifier));
    }

    void accept(ExpressionVisitor<void>& visitor) override
    {
        return visitor.visit_name_of(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return keyword;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return identifier;
    }

    [[nodiscard]] std::string get_text() const override
    {
        return "nameof " + identifier.get_text();
    }
};