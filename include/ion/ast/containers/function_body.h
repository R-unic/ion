#pragma once
#include "expression_body.h"

struct FunctionBody final : SyntaxNode
{
    std::optional<ExpressionBody*> expression_body;
    /** Must be a Block */
    std::optional<statement_ptr_t> block;

    FunctionBody(const FunctionBody&) = delete;
    FunctionBody& operator=(const FunctionBody&) = delete;

    FunctionBody(const std::optional<ExpressionBody*> expression_body, std::optional<statement_ptr_t> block)
        : expression_body(expression_body),
          block(std::move(block))
    {
    }

    ~FunctionBody() override
    {
        if (expression_body.has_value())
            delete *expression_body;
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return block.has_value() ? block.value()->get_first_token() : expression_body.value()->get_first_token();
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return block.has_value() ? block.value()->get_last_token() : expression_body.value()->get_last_token();
    }

    [[nodiscard]] FileSpan get_span() const override
    {
        return block.has_value() ? block.value()->get_span() : expression_body.value()->get_span();
    }

    [[nodiscard]] std::string get_text() const override
    {
        return block.has_value() ? block.value()->get_text() : expression_body.value()->get_text();
    }
};