#pragma once
#include <algorithm>

#include "ion/token.h"
#include "ion/ast/node.h"

class Return final : public Statement
{
public:
    Token return_keyword;
    std::optional<expression_ptr_t> expression;

    explicit Return(Token return_keyword, std::optional<expression_ptr_t> expression)
        : return_keyword(std::move(return_keyword)),
          expression(std::move(expression))
    {
    }

    static statement_ptr_t create(Token return_keyword, std::optional<expression_ptr_t> expression)
    {
        return std::make_unique<Return>(std::move(return_keyword), std::move(expression));
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_return(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return return_keyword;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return expression.has_value() ? expression.value()->get_last_token() : return_keyword;;
    }

    [[nodiscard]] std::string get_text() const override
    {
        return return_keyword.get_text() + (expression.has_value() ? ' ' + expression.value()->get_text() : "");
    }

    [[nodiscard]] bool is_return() const override
    {
        return true;
    }
};