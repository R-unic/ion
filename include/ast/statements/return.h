#pragma once
#include <algorithm>

#include "ast/node.h"

class Return final : public Statement
{
public:
    Token keyword;
    std::optional<expression_ptr_t> expression;
    
    explicit Return(Token keyword, std::optional<expression_ptr_t> expression)
        : keyword(std::move(keyword)), expression(std::move(expression))
    {
    }
    
    static statement_ptr_t create(Token keyword, std::optional<expression_ptr_t> expression)
    {
        return std::make_unique<Return>(std::move(keyword), std::move(expression));
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_return(*this);
    }

    [[nodiscard]] FileSpan get_span() const override
    {
        const auto end_location = expression.has_value() ? expression.value()->get_span().end : keyword.span.end;
        return create_span(keyword.span.start, end_location);
    }
    
    [[nodiscard]] std::string get_text() const override
    {
        return keyword.get_text() + (expression.has_value() ? ' ' + expression.value()->get_text() : "");
    }
};